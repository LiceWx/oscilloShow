#include "pack.h"
#include "constructor.h"
#include "preview.h"
#include "canny.h"
#include "../drivers/bmp_handler.h"
#include "../drivers/wav_handler.h"
#include <bits/stdc++.h>
#include <fstream>
#include <sys/stat.h>

std::vector<int> finalCompressed[2]; // Store final compressed result

// Function to create directory if it doesn't exist
void create_directory_if_not_exists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        // Directory doesn't exist, create it
        #ifdef _WIN32
            system(("mkdir \"" + path + "\"").c_str());
        #else
            system(("mkdir -p \"" + path + "\"").c_str());
        #endif
    }
}

// Function to write info data directly to play.bin file
void write_info_to_play_bin(bool is_gif) {
    // Create SDfiles directory if it doesn't exist
    create_directory_if_not_exists("D:/OscilloProj/SDfiles");
    
    std::ofstream output_file("D:/OscilloProj/SDfiles/play.bin", std::ios::binary);
    
    if (is_gif) {
        // For GIF files, try to read existing info.txt
        std::string info_path = "D:/OscilloProj/SDfiles/info.txt";
        std::ifstream info_file(info_path, std::ios::binary);
        
        if (info_file.is_open()) {
            // Read and copy entire info.txt file content
            info_file.seekg(0, std::ios::end);
            std::streamsize size = info_file.tellg();
            info_file.seekg(0, std::ios::beg);
            
            if (size > 0) {
                std::vector<char> info_data(size);
                if (info_file.read(info_data.data(), size)) {
                    output_file.write(info_data.data(), size);
                    std::cout << "Added info.txt content (" << size << " bytes) to play.bin" << std::endl;
                }
            }
            info_file.close();
        } else {
            std::cout << "Warning: info.txt not found for GIF processing" << std::endl;
        }
    } else {
        // For BMP files, create info data: FPS=0 (2 bytes), framecount=1 (2 bytes), framesize (2 bytes)
        uint16_t fps = 0;        // FPS = 0 for BMP
        uint16_t framecount = 1; // framecount = 1 for BMP
        uint16_t frame_size_16 = static_cast<uint16_t>(frameSize); // framesize for BMP
        
        output_file.write(reinterpret_cast<const char*>(&fps), sizeof(uint16_t));
        output_file.write(reinterpret_cast<const char*>(&framecount), sizeof(uint16_t));
        output_file.write(reinterpret_cast<const char*>(&frame_size_16), sizeof(uint16_t));
        
        std::cout << "Added BMP info data (FPS=0, framecount=1, framesize=" << frameSize << ") to play.bin" << std::endl;
    }
    
    output_file.close();
}

// Function to append frame data directly to play.bin file
void append_frame_to_play_bin(int m, int frame_id) {
    (void)frame_id; // Suppress unused parameter warning
    int n = signalXY.size();
    int height = grayMatrix.size();
    int width = grayMatrix[0].size();
    int scale = std::max(height, width);
    
    // Open file in append mode
    std::ofstream output_file("D:/OscilloProj/SDfiles/play.bin", std::ios::binary | std::ios::app);
    
    // Pack X coordinates
    for (int i = 0; i < m; i++) {
        int idx = int(1.0 * n / m * i + 0.5);
        int x = signalXY[idx].first;
        x = x * 1.0 / scale * (1 << 12);
        int u = x >> 8 & 0x0F;
        int v = x & 0xFF;
        output_file << (char) v << (char) u;
    }
    // Pack Y coordinates
    for (int i = 0; i < m; i++) {
        int idx = int(1.0 * n / m * i + 0.5);
        int y = signalXY[idx].second;
        y = y * 1.0 / scale * (1 << 12);
        int u = y >> 8 & 0x0F;
        int v = y & 0xFF;
        output_file << (char) v << (char) u;
    }
    
    output_file.close();
    std::cout << "Appended frame " << frame_id << " data (" << (m * 4) << " bytes) to play.bin" << std::endl;
}

// Common function to pack BMP signal data
void pack_bmp_signal(int m, vector<int> compressed[2]) {
    int n = signalXY.size();
    if (n == 0) return;
    
    int height = grayMatrix.size();
    int width = grayMatrix[0].size();
    int scale = std::max(height, width);
    
    for (int i = 0; i < m; i++) {
        int idx = int(1.0 * n / m * i + 0.5);
        int x = signalXY[idx].first;
        int y = signalXY[idx].second;
        x = x * 1.0 / scale * (1 << 16) - (1 << 15);
        y = y * 1.0 / scale * (1 << 16) - (1 << 15);
        compressed[0].push_back(x);
        compressed[1].push_back(y);
    }
}

// Function to update framesize in info.txt for GIF files
void update_gif_info_framesize() {
    std::string info_path = "D:/OscilloProj/SDfiles/info.txt";
    std::fstream info_file(info_path, std::ios::binary | std::ios::in | std::ios::out);
    
    if (info_file.is_open()) {
        // Seek to position 4 (after FPS and framecount, each 2 bytes)
        info_file.seekp(4, std::ios::beg);
        
        // Write framesize as 16-bit binary
        uint16_t frame_size_16 = static_cast<uint16_t>(frameSize);
        info_file.write(reinterpret_cast<const char*>(&frame_size_16), sizeof(uint16_t));
        
        info_file.close();
        std::cout << "Updated framesize=" << frameSize << " in info.txt" << std::endl;
    } else {
        std::cout << "Warning: Could not update framesize in info.txt" << std::endl;
    }
}

// Function to initialize play.bin file for GIF processing
void initialize_play_bin_for_gif() {
    // Update framesize in info.txt first
    update_gif_info_framesize();
    
    // Then write updated info.txt content to play.bin
    write_info_to_play_bin(true);
}
// Function to finalize play.bin file (just print completion message)
void finalize_play_bin() {
    std::ifstream file("D:/OscilloProj/SDfiles/play.bin", std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streamsize size = file.tellg();
        file.close();
        std::cout << "SD file saved to: D:/OscilloProj/SDfiles/play.bin (size: " 
                  << size << " bytes)" << std::endl;
    }
}

void compress_and_append_frame(int m, int frame_id) {
    if (signalXY.size() == 0) return;
    
    vector<int> compressed[2];
    pack_bmp_signal(m, compressed);
    
    // Append frame data directly to play.bin file
    append_frame_to_play_bin(m, frame_id);
    
    // Append to final compressed data
    finalCompressed[0].insert(finalCompressed[0].end(), compressed[0].begin(), compressed[0].end());
    finalCompressed[1].insert(finalCompressed[1].end(), compressed[1].begin(), compressed[1].end());
}

void pack_signal(int m) {
    int height = grayMatrix.size();
    int width = grayMatrix[0].size(); 

    // For single BMP files, use the original algorithm
    if (signalXY.size() < m)
        m = signalXY.size();
    if (finalCompressed[0].empty()) {        
        vector<int> compressed[2];
        pack_bmp_signal(m, compressed);

        // For single BMP files, write info data first (FPS=0, framecount=1)
        write_info_to_play_bin(false);
        
        // Then append frame data
        append_frame_to_play_bin(m, 0);
        
        // Finalize the file
        finalize_play_bin();

        vector<int> copy_compressed[2];
        copy_compressed[0] = compressed[0];
        copy_compressed[1] = compressed[1];
        for (int i = 0; i < 24; i++) {
            copy_compressed[0].insert(copy_compressed[0].end(), compressed[0].begin(), compressed[0].end());
            copy_compressed[1].insert(copy_compressed[1].end(), compressed[1].begin(), compressed[1].end());
        }
        createStereoWAV(copy_compressed[0], copy_compressed[1], 48000, BitDepth::BIT_16, "play.wav");
    } else {
        // For GIF files, use the accumulated compressed data
        createStereoWAV(finalCompressed[0], finalCompressed[1], 48000, BitDepth::BIT_16, "play.wav");
        
        // Note: For GIF files, info.txt should have been written first before any frame data
        // Frame data has already been accumulated through compress_and_append_frame calls
        // Finalize the file
        finalize_play_bin();
    }
}
