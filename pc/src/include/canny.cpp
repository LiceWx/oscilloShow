#include "canny.h"
#include "../drivers/bmp_handler.h"
#include <bits/stdc++.h>
#include <cstdlib>

const double PI = acos(-1);

grayMatrix_t grayMatrix;

// RGB to grayscale conversion using standard luminance formula
uint8_t rgbToGrayscale(const PixelRGB& pixel) {
    return static_cast<uint8_t>(0.299 * pixel.r + 0.587 * pixel.g + 0.114 * pixel.b);
}

// Convert color image data to grayscale matrix
grayMatrix_t convertToGrayscaleMatrix(const vector<vector<PixelRGB>>& colorData) {
    if (colorData.empty()) {
        return grayMatrix_t();
    }
    
    int height = colorData.size();
    int width = colorData[0].size();
    grayMatrix_t grayMatrix(height, vector<uint8_t>(width));
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            grayMatrix[y][x] = rgbToGrayscale(colorData[y][x]);
        }
    }
    
    return grayMatrix;
}

void save_matrix_to_file(const grayMatrix_t& matrix, const string& filename) {
    std::ofstream fout(filename);
    for (size_t i = 0; i < matrix.size(); i++) {
        for (size_t j = 0; j < matrix[i].size(); j++) {
            fout << (int)matrix[i][j] << " ";
        }
        fout << std::endl;
    }
    fout.close();
}

bool initialize(string inputFile) {
    BMPImage img;
    if (!img.readBMP(inputFile)) {
        return 1;
    }
        
    if (img.isGrayscaleImage()) {
        grayMatrix = img.toGrayscaleMatrix();
    } else {
        const auto& colorData = img.getColorData();
        grayMatrix = convertToGrayscaleMatrix(colorData);
    }
    
    if (grayMatrix.empty()) {
        return 1;
    }
    
    // Save matrix to text file
    // save_matrix_to_file(grayMatrix, "grayMatrix.txt");
    img.fromGrayscaleMatrix(grayMatrix);
    img.writeBMP("gray.bmp");

    std::cout << "image height: " << grayMatrix.size() << std::endl;
    std::cout << "image width: " << grayMatrix[0].size() << std::endl;
    
    return 0;
}

const kernel_t gauss_kernel_3 = {
    {1.0/16, 2.0/16, 1.0/16},
    {2.0/16, 4.0/16, 2.0/16},
    {1.0/16, 2.0/16, 1.0/16}
};
const kernel_t gauss_kernel_5 = {
    {1.0/273, 4.0/273, 7.0/273, 4.0/273, 1.0/273},
    {4.0/273, 16.0/273, 26.0/273, 16.0/273, 4.0/273},
    {7.0/273, 26.0/273, 41.0/273, 26.0/273, 7.0/273},
    {4.0/273, 16.0/273, 26.0/273, 16.0/273, 4.0/273},
    {1.0/273, 4.0/273, 7.0/273, 4.0/273, 1.0/273}
};

// 2D convolution function using standard convolution definition
// Template function that accepts any input matrix type and kernel type, returns double matrix
template<typename InputType, typename KernelType>
matrix<double> convolution2d(const matrix<InputType>& input, const matrix<KernelType>& kernel) {
    if (input.empty() || kernel.empty()) {
        return matrix<double>();
    }
    
    int input_height = input.size();
    int input_width = input[0].size();
    int kernel_height = kernel.size();
    int kernel_width = kernel[0].size();
    
    // Calculate output dimensions (valid convolution - no padding)
    int output_height = input_height - kernel_height + 1;
    int output_width = input_width - kernel_width + 1;
    
    // Handle edge case where kernel is larger than input
    if (output_height <= 0 || output_width <= 0) {
        return matrix<double>();
    }
    
    matrix<double> result(output_height, vector<double>(output_width));
    
    // Standard 2D convolution using flipped kernel
    for (int i = 0; i < output_height; i++) {
        for (int j = 0; j < output_width; j++) {
            double sum = 0.0;
            
            for (int ki = 0; ki < kernel_height; ki++) {
                for (int kj = 0; kj < kernel_width; kj++) {
                    // Standard convolution: flip kernel indices
                    int kernel_i = kernel_height - 1 - ki;
                    int kernel_j = kernel_width - 1 - kj;
                    
                    sum += static_cast<double>(input[i + ki][j + kj]) * static_cast<double>(kernel[kernel_i][kernel_j]);
                }
            }
            
            result[i][j] = sum;
        }
    }
    
    return result;
}

// Simplified gaussian blur using convolution
grayMatrix_t gaussian_blur(const grayMatrix_t& input, const kernel_t& gaussian_kernel) {
    auto result_double = convolution2d(input, gaussian_kernel);
    
    if (result_double.empty()) {
        return grayMatrix_t();
    }
    
    int height = result_double.size();
    int width = result_double[0].size();
    grayMatrix_t result(height, vector<uint8_t>(width));
    
    // Convert double matrix back to uint8_t matrix with clamping
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            result[i][j] = static_cast<uint8_t>(std::max(0.0, std::min(255.0, result_double[i][j])));
        }
    }
    
    return result;
}

const kernel_t sobel_x_kernel = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};
const kernel_t sobel_y_kernel = {
    {-1, -2, -1},
    { 0,  0,  0},
    { 1,  2,  1}
};
const kernel_t prewitt_x_kernel = {
    {-1, 0, 1},
    {-1, 0, 1},
    {-1, 0, 1}
};
const kernel_t prewitt_y_kernel = {
    {-1, -1, -1},
    { 0,  0,  0},
    { 1,  1,  1}
};

std::pair<realMatrix_t, realMatrix_t> filter(const grayMatrix_t& input, const kernel_t& x_kernel, const kernel_t& y_kernel) {
    auto x = convolution2d(input, x_kernel);
    auto y = convolution2d(input, y_kernel);

    if (x.empty() || y.empty()) {
        return std::make_pair(realMatrix_t(), realMatrix_t());
    }

    size_t height = x.size();
    size_t width = x[0].size();
    
    realMatrix_t magnitude(height, vector<double>(width));
    realMatrix_t direction(height, vector<double>(width));
    
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            double dx = x[i][j];  // Already double from convolution2d
            double dy = y[i][j];  // Already double from convolution2d
            magnitude[i][j] = std::sqrt(dx * dx + dy * dy);
            direction[i][j] = std::atan2(dy, dx) * 180 / PI;
            if (direction[i][j] < 0) {
                direction[i][j] += 180;
            }
        }
    }
    return std::make_pair(magnitude, direction);
}

realMatrix_t non_maximum_suppression(const realMatrix_t& magnitude, const realMatrix_t& direction) {
    int height = magnitude.size();
    int width = magnitude[0].size();
    realMatrix_t result(height, vector<double>(width));

    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            double q = 255, r = 255;
            if ((0 <= direction[i][j] && direction[i][j] < 22.5) || (157.5 <= direction[i][j] && direction[i][j] <= 180)) {
                q = magnitude[i][j + 1];
                r = magnitude[i][j - 1];
            } else if (22.5 <= direction[i][j] && direction[i][j] < 67.5) {
                q = magnitude[i + 1][j - 1];
                r = magnitude[i - 1][j + 1];
            } else if (67.5 <= direction[i][j] && direction[i][j] < 112.5) {
                q = magnitude[i + 1][j];
                r = magnitude[i - 1][j];
            } else if (112.5 <= direction[i][j] && direction[i][j] < 157.5) {
                q = magnitude[i - 1][j - 1];
                r = magnitude[i + 1][j + 1];
            }
            if (magnitude[i][j] >= q && magnitude[i][j] >= r) {
                result[i][j] = magnitude[i][j];
            } else {
                result[i][j] = 0;
            }
        }
    }
    
    return result;
}

grayMatrix_t double_threshold(const realMatrix_t& input, double low_threshold, double high_threshold) {
    int height = input.size();
    int width = input[0].size();
    grayMatrix_t result(height, vector<uint8_t>(width));

    double max_value = 0;
    double min_value = 255;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            max_value = std::max(max_value, input[i][j]);
            min_value = std::min(min_value, input[i][j]);
        }
    }
    low_threshold = min_value + (max_value - min_value) * low_threshold;
    high_threshold = min_value + (max_value - min_value) * high_threshold;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (input[i][j] >= high_threshold) {
                result[i][j] = 255;
            } else if (input[i][j] >= low_threshold) {
                result[i][j] = 128;
            } else {
                result[i][j] = 0;
            }
        }
    }

    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            if (result[i][j] == 128) {
                if (result[i - 1][j - 1] == 255 || result[i - 1][j] == 255 || result[i - 1][j + 1] == 255 || result[i][j - 1] == 255 || result[i][j + 1] == 255 || result[i + 1][j - 1] == 255 || result[i + 1][j] == 255 || result[i + 1][j + 1] == 255) {
                    result[i][j] = 255;
                } else {
                    result[i][j] = 0;
                }
            }
        }
    }

    return result;
}

bool single_color() {
    int height = grayMatrix.size();
    int width = grayMatrix[0].size();
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (grayMatrix[i][j] != grayMatrix[0][0]) {
                return false;
            }
        }
    }
    return true;
}

void canny(string srcFile, double highThreshold, double lowThreshold) {
    initialize(srcFile);

    if (single_color()) {
        grayMatrix_t result(grayMatrix.size(), vector<uint8_t>(grayMatrix[0].size(), 0));
        result[0][0] = 255;
        grayMatrix = result;
        return;
    } else {
        grayMatrix = gaussian_blur(grayMatrix, gauss_kernel_3);

        std::pair<realMatrix_t, realMatrix_t> result = filter(grayMatrix, sobel_x_kernel, sobel_y_kernel);
        realMatrix_t magnitude = result.first;
        realMatrix_t direction = result.second;

        realMatrix_t suppressed = non_maximum_suppression(magnitude, direction);

        grayMatrix_t double_threshold_result = double_threshold(suppressed, lowThreshold, highThreshold);
        grayMatrix = double_threshold_result;
    }

    // Save as BMP file
    BMPImage outputImg;
    outputImg.fromGrayscaleMatrix(grayMatrix);
    outputImg.writeBMP("canny.bmp");
    std::cout << "Result canny.bmp saved." << std::endl;
}
