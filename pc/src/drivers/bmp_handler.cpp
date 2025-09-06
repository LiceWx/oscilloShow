#include "bmp_handler.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>

BMPImage::BMPImage() : isGrayscale(false) {
    // 初始化文件头
    memset(&fileHeader, 0, sizeof(fileHeader));
    memset(&infoHeader, 0, sizeof(infoHeader));
    fileHeader.fileType = 0x4D42; // "BM"
    infoHeader.size = 40;
    infoHeader.planes = 1;
}

BMPImage::~BMPImage() {
    // 析构函数
}

uint32_t BMPImage::calculateRowSize(int width, int bitsPerPixel) const {
    // BMP 行大小必须是4字节对齐
    return ((width * bitsPerPixel + 31) / 32) * 4;
}

bool BMPImage::readBMP(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        return false;
    }

    // 读取文件头
    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    if (fileHeader.fileType != 0x4D42) {
        std::cerr << "Error: Not a valid BMP file" << std::endl;
        return false;
    }

    // 读取信息头
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
    
    // 检查支持的格式
    if (infoHeader.compression != 0) {
        std::cerr << "Error: Compressed BMP files are not supported" << std::endl;
        return false;
    }

    // 判断是否为灰度图像
    isGrayscale = (infoHeader.bitsPerPixel == 8);

    if (isGrayscale) {
        // 读取调色板（灰度图像有256色调色板）
        colorTable.resize(256);
        file.read(reinterpret_cast<char*>(colorTable.data()), 256 * sizeof(RGBQuad));
        readGrayscaleData(file);
    } else if (infoHeader.bitsPerPixel == 24) {
        // 24位彩色图像
        readColorData(file);
    } else {
        std::cerr << "Error: Unsupported bits per pixel: " << infoHeader.bitsPerPixel << std::endl;
        return false;
    }

    file.close();
    return true;
}

void BMPImage::readGrayscaleData(std::ifstream& file) {
    int width = infoHeader.width;
    int height = abs(infoHeader.height);
    uint32_t rowSize = calculateRowSize(width, 8);
    
    grayData.resize(height);
    for (int i = 0; i < height; i++) {
        grayData[i].resize(width);
    }

    // 移动到图像数据开始位置
    file.seekg(fileHeader.offsetData);

    std::vector<uint8_t> rowBuffer(rowSize);
    
    // BMP图像数据是从底部到顶部存储的
    for (int y = height - 1; y >= 0; y--) {
        file.read(reinterpret_cast<char*>(rowBuffer.data()), rowSize);
        for (int x = 0; x < width; x++) {
            grayData[y][x] = rowBuffer[x];
        }
    }
}

void BMPImage::readColorData(std::ifstream& file) {
    int width = infoHeader.width;
    int height = abs(infoHeader.height);
    uint32_t rowSize = calculateRowSize(width, 24);
    
    colorData.resize(height);
    for (int i = 0; i < height; i++) {
        colorData[i].resize(width);
    }

    // 移动到图像数据开始位置
    file.seekg(fileHeader.offsetData);

    std::vector<uint8_t> rowBuffer(rowSize);
    
    // BMP图像数据是从底部到顶部存储的
    for (int y = height - 1; y >= 0; y--) {
        file.read(reinterpret_cast<char*>(rowBuffer.data()), rowSize);
        for (int x = 0; x < width; x++) {
            // BMP中BGR格式存储
            int offset = x * 3;
            colorData[y][x].b = rowBuffer[offset];
            colorData[y][x].g = rowBuffer[offset + 1];
            colorData[y][x].r = rowBuffer[offset + 2];
        }
    }
}

bool BMPImage::writeBMP(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create file: " << filename << std::endl;
        return false;
    }

    // 计算文件大小
    int width = infoHeader.width;
    int height = abs(infoHeader.height);
    uint32_t rowSize;
    uint32_t imageSize;
    uint32_t paletteSize = 0;

    if (isGrayscale) {
        rowSize = calculateRowSize(width, 8);
        imageSize = rowSize * height;
        paletteSize = 256 * sizeof(RGBQuad);
    } else {
        rowSize = calculateRowSize(width, 24);
        imageSize = rowSize * height;
    }

    // 更新文件头
    BMPFileHeader writeFileHeader = fileHeader;
    BMPInfoHeader writeInfoHeader = infoHeader;
    
    writeFileHeader.offsetData = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + paletteSize;
    writeFileHeader.fileSize = writeFileHeader.offsetData + imageSize;
    writeInfoHeader.sizeImage = imageSize;

    // 写入文件头和信息头
    file.write(reinterpret_cast<const char*>(&writeFileHeader), sizeof(writeFileHeader));
    file.write(reinterpret_cast<const char*>(&writeInfoHeader), sizeof(writeInfoHeader));

    if (isGrayscale) {
        // 写入调色板
        file.write(reinterpret_cast<const char*>(colorTable.data()), paletteSize);
        writeGrayscaleData(file);
    } else {
        writeColorData(file);
    }

    file.close();
    return true;
}

void BMPImage::writeGrayscaleData(std::ofstream& file) const {
    int width = infoHeader.width;
    int height = abs(infoHeader.height);
    uint32_t rowSize = calculateRowSize(width, 8);
    
    std::vector<uint8_t> rowBuffer(rowSize, 0);

    // BMP图像数据是从底部到顶部存储的
    for (int y = height - 1; y >= 0; y--) {
        std::fill(rowBuffer.begin(), rowBuffer.end(), 0);
        for (int x = 0; x < width; x++) {
            rowBuffer[x] = grayData[y][x];
        }
        file.write(reinterpret_cast<const char*>(rowBuffer.data()), rowSize);
    }
}

void BMPImage::writeColorData(std::ofstream& file) const {
    int width = infoHeader.width;
    int height = abs(infoHeader.height);
    uint32_t rowSize = calculateRowSize(width, 24);
    
    std::vector<uint8_t> rowBuffer(rowSize, 0);

    // BMP图像数据是从底部到顶部存储的
    for (int y = height - 1; y >= 0; y--) {
        std::fill(rowBuffer.begin(), rowBuffer.end(), 0);
        for (int x = 0; x < width; x++) {
            // BMP中BGR格式存储
            int offset = x * 3;
            rowBuffer[offset] = colorData[y][x].b;
            rowBuffer[offset + 1] = colorData[y][x].g;
            rowBuffer[offset + 2] = colorData[y][x].r;
        }
        file.write(reinterpret_cast<const char*>(rowBuffer.data()), rowSize);
    }
}

std::vector<std::vector<uint8_t>> BMPImage::toGrayscaleMatrix() const {
    if (!isGrayscale) {
        std::cerr << "Warning: Current image is not an 8-bit grayscale image" << std::endl;
        return std::vector<std::vector<uint8_t>>();
    }
    return grayData;
}

void BMPImage::fromGrayscaleMatrix(const std::vector<std::vector<uint8_t>>& matrix) {
    if (matrix.empty() || matrix[0].empty()) {
        std::cerr << "Error: Input matrix is empty" << std::endl;
        return;
    }

    int height = matrix.size();
    int width = matrix[0].size();
    
    // 设置为灰度模式
    isGrayscale = true;
    grayData = matrix;
    colorData.clear();

    // 更新信息头
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.bitsPerPixel = 8;
    infoHeader.compression = 0;

    // 创建灰度调色板
    colorTable.resize(256);
    for (int i = 0; i < 256; i++) {
        colorTable[i].red = i;
        colorTable[i].green = i;
        colorTable[i].blue = i;
        colorTable[i].reserved = 0;
    }

    // 计算图像大小
    uint32_t rowSize = calculateRowSize(width, 8);
    infoHeader.sizeImage = rowSize * height;
    
    // 计算文件大小
    uint32_t paletteSize = 256 * sizeof(RGBQuad);
    fileHeader.offsetData = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + paletteSize;
    fileHeader.fileSize = fileHeader.offsetData + infoHeader.sizeImage;
}

void BMPImage::fromColorMatrix(const std::vector<std::vector<PixelRGB>>& matrix) {
    if (matrix.empty() || matrix[0].empty()) {
        std::cerr << "Error: Input matrix is empty" << std::endl;
        return;
    }

    int height = matrix.size();
    int width = matrix[0].size();
    
    // 设置为彩色模式
    isGrayscale = false;
    colorData = matrix;
    grayData.clear();
    colorTable.clear();

    // 更新信息头
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.bitsPerPixel = 24;
    infoHeader.compression = 0;

    // 计算图像大小
    uint32_t rowSize = calculateRowSize(width, 24);
    infoHeader.sizeImage = rowSize * height;
    
    // 计算文件大小
    fileHeader.offsetData = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    fileHeader.fileSize = fileHeader.offsetData + infoHeader.sizeImage;
}
