#include "drivers/bmp_handler.h"
#include "drivers/wav_handler.h"
#include "include/canny.h"
#include "include/constructor.h"
#include "include/preview.h"
#include "include/pack.h"
#include <bits/stdc++.h>
#include <cstdlib>
#include <dirent.h>
#include <sys/stat.h>

int frameSize;

bool is_gif_file(const string& filename) {
    string ext = filename.substr(filename.find_last_of('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == "gif";
}

bool file_exists(const string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

vector<string> get_bmp_files(const string& directory) {
    vector<string> bmpFiles;
    DIR* dir = opendir(directory.c_str());
    if (dir == nullptr) return bmpFiles;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string filename = entry->d_name;
        if (filename.length() > 4 && filename.substr(filename.length() - 4) == ".bmp") {
            bmpFiles.push_back(directory + "/" + filename);
        }
    }
    closedir(dir);
    
    std::sort(bmpFiles.begin(), bmpFiles.end());
    return bmpFiles;
}

void process_gif(const string& gifFile) {
    // Get threshold values from user
    double highThreshold = 0.02;
    double lowThreshold = 0.01;
    std::cout << "Entry high and low threshold (0.0 - 1.0): ";
    std::cin >> highThreshold >> lowThreshold;

    char flag = 'n';
    std::cout << "Already converted gif to bmp frames? (y/n): ";
    std::cin >> flag;
    if (flag == 'n' || flag == 'N') {    
        // Convert GIF to BMP frames
        string command = "python src/gif_to_bmp.py " + gifFile;
        int result = std::system(command.c_str());
        if (result != 0) {
            std::cerr << "Error converting GIF to BMP frames" << std::endl;
            return;
        }
    }
    
    // Process each BMP frame
    const string framesDir = "D:/OscilloProj/frames";
    vector<string> bmpFiles = get_bmp_files(framesDir);
    
    if (bmpFiles.empty()) {
        std::cerr << "No BMP files found in frames directory" << std::endl;
        return;
    }

    flag = 'n';
    std::cout << "Keep wav file? (y/n): ";
    std::cin >> flag;
    
    finalCompressed[0].clear();
    finalCompressed[1].clear();
    
    // Initialize play.bin file with info data for GIF processing
    initialize_play_bin_for_gif();
    
    int frame_id = 0;
    for (const string& bmpFile : bmpFiles) {
        std::cout << "Processing " << bmpFile << " (frame " << frame_id << ")" << std::endl;
        
        // Reset all global variables
        points.clear();
        edges.clear();
        signalXY.clear();
        visited.clear();
        belong.clear();
        dfn.clear();
        dist.clear();
        mst.clear();
        
        canny(bmpFile, highThreshold, lowThreshold);
        construct_signal();
        
        // Compress this frame and append to final result
        compress_and_append_frame(frameSize, frame_id);
        frame_id++;

        if (flag == 'y' || flag == 'Y') {
            finalCompressed[0].clear();
            finalCompressed[1].clear();
        }
    }
    
    std::cout << "Total compressed signal length: " << finalCompressed[0].size() << std::endl;
}

void process_bmp(const string& bmpFile) {
    double highThreshold = 0.02;
    double lowThreshold = 0.01;
    std::cout << "Entry high and low threshold (0.0 - 1.0): ";
    std::cin >> highThreshold >> lowThreshold;
    
    canny(bmpFile, highThreshold, lowThreshold);
    
    // Confirmation step for BMP files
    char confirm;
    std::cout << "Canny edge detection completed. Check canny.bmp file." << std::endl;
    std::cout << "Continue with current result? (y/n): ";
    std::cin >> confirm;
    
    if (confirm == 'n' || confirm == 'N') {
        std::cout << "Please modify canny.bmp manually if needed, then press Enter to continue...";
        std::cin.ignore();
        std::cin.get();
        
        // Reload canny.bmp and update grayMatrix
        BMPImage cannyImg;
        if (cannyImg.readBMP("canny.bmp")) {
            if (cannyImg.isGrayscaleImage()) {
                grayMatrix = cannyImg.toGrayscaleMatrix();
            } else {
                // Convert to grayscale if needed
                const auto& colorData = cannyImg.getColorData();
                grayMatrix = convertToGrayscaleMatrix(colorData);
            }
            std::cout << "Reloaded canny.bmp successfully." << std::endl;
            std::cout << "Updated image size: " << grayMatrix.size() << "x" << grayMatrix[0].size() << std::endl;
        } else {
            std::cerr << "Error: Failed to reload canny.bmp" << std::endl;
            return;
        }
    }
    
    construct_signal();
    preview_signal();
}

int main() {
    string srcFile;
    std::cout << "Entry source file (bmp or gif): ";
    std::cin >> srcFile;
    
    std::cout << "Entry frame size: ";
    std::cin >> frameSize;

    if (!file_exists(srcFile)) {
        std::cerr << "File not found: " << srcFile << std::endl;
        return 1;
    }
    
    if (is_gif_file(srcFile)) {
        process_gif(srcFile);
    } else {
        process_bmp(srcFile);
    }
    
    pack_signal(frameSize);

    // std::cout << "max_cnt: " << max_cnt << std::endl;
    return 0;
}