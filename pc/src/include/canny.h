#ifndef CANNY_H
#define CANNY_H

#include <vector>
#include <string>
#include <cstdint>

using std::vector;
using std::string;

template<typename T>
using matrix = vector<vector<T>>;

using grayMatrix_t = matrix<uint8_t>;
using kernel_t = matrix<double>;
using realMatrix_t = matrix<double>;

// External global variable
extern grayMatrix_t grayMatrix;

// Function declarations
uint8_t rgbToGrayscale(const struct PixelRGB& pixel);
grayMatrix_t convertToGrayscaleMatrix(const vector<vector<struct PixelRGB>>& colorData);
void save_matrix_to_file(const grayMatrix_t& matrix, const string& filename);
bool initialize(string inputFile);

// Convolution and filtering functions
template<typename InputType, typename KernelType>
matrix<double> convolution2d(const matrix<InputType>& input, const matrix<KernelType>& kernel);

grayMatrix_t gaussian_blur(const grayMatrix_t& input, const kernel_t& gaussian_kernel);
std::pair<realMatrix_t, realMatrix_t> filter(const grayMatrix_t& input, const kernel_t& x_kernel, const kernel_t& y_kernel);
realMatrix_t non_maximum_suppression(const realMatrix_t& magnitude, const realMatrix_t& direction);
grayMatrix_t double_threshold(const realMatrix_t& input, double low_threshold, double high_threshold);

// Main Canny function
void canny(string srcFile, double highThreshold = 0.02, double lowThreshold = 0.01);

// Kernels
extern const kernel_t gauss_kernel_3;
extern const kernel_t gauss_kernel_5;
extern const kernel_t sobel_x_kernel;
extern const kernel_t sobel_y_kernel;
extern const kernel_t prewitt_x_kernel;
extern const kernel_t prewitt_y_kernel;

#endif // CANNY_H
