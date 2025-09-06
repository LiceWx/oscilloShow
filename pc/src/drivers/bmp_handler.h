#ifndef BMP_HANDLER_H
#define BMP_HANDLER_H

#include <vector>
#include <string>
#include <cstdint>

// BMP 文件头结构
#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t fileType = 0x4D42;      // "BM"
    uint32_t fileSize = 0;           // 文件大小
    uint16_t reserved1 = 0;          // 保留字段
    uint16_t reserved2 = 0;          // 保留字段
    uint32_t offsetData = 0;         // 图像数据偏移量
};

struct BMPInfoHeader {
    uint32_t size = 40;              // 信息头大小
    int32_t width = 0;               // 图像宽度
    int32_t height = 0;              // 图像高度
    uint16_t planes = 1;             // 颜色平面数
    uint16_t bitsPerPixel = 0;       // 每像素位数
    uint32_t compression = 0;        // 压缩方式
    uint32_t sizeImage = 0;          // 图像数据大小
    int32_t xPixelsPerMeter = 0;     // 水平分辨率
    int32_t yPixelsPerMeter = 0;     // 垂直分辨率
    uint32_t colorsUsed = 0;         // 使用的颜色数
    uint32_t colorsImportant = 0;    // 重要颜色数
};

struct RGBQuad {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t reserved;
};
#pragma pack(pop)

// 像素数据结构
struct PixelRGB {
    uint8_t r, g, b;
    PixelRGB() : r(0), g(0), b(0) {}
    PixelRGB(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
};

// BMP 图像类
class BMPImage {
private:
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    std::vector<RGBQuad> colorTable;
    std::vector<std::vector<uint8_t>> grayData;    // 灰度数据
    std::vector<std::vector<PixelRGB>> colorData;  // 彩色数据
    bool isGrayscale;

    // 辅助函数
    uint32_t calculateRowSize(int width, int bitsPerPixel) const;
    void readGrayscaleData(std::ifstream& file);
    void readColorData(std::ifstream& file);
    void writeGrayscaleData(std::ofstream& file) const;
    void writeColorData(std::ofstream& file) const;

public:
    BMPImage();
    ~BMPImage();

    // 读取 BMP 文件
    bool readBMP(const std::string& filename);
    
    // 输出 BMP 文件
    bool writeBMP(const std::string& filename) const;
    
    // 将8位灰度图像转换为 vector<vector<uint8_t>> 格式
    std::vector<std::vector<uint8_t>> toGrayscaleMatrix() const;
    
    // 从 vector<vector<uint8_t>> 创建灰度图像
    void fromGrayscaleMatrix(const std::vector<std::vector<uint8_t>>& matrix);
    
    // 从 vector<vector<PixelRGB>> 创建彩色图像
    void fromColorMatrix(const std::vector<std::vector<PixelRGB>>& matrix);
    
    // 获取图像信息
    int getWidth() const { return infoHeader.width; }
    int getHeight() const { return abs(infoHeader.height); }
    int getBitsPerPixel() const { return infoHeader.bitsPerPixel; }
    bool isGrayscaleImage() const { return isGrayscale; }
    
    // 获取像素数据
    const std::vector<std::vector<uint8_t>>& getGrayscaleData() const { return grayData; }
    const std::vector<std::vector<PixelRGB>>& getColorData() const { return colorData; }
};

#endif // BMP_HANDLER_H
