#ifndef WAV_HANDLER_H
#define WAV_HANDLER_H

#include <vector>
#include <string>
#include <cstdint>

// WAV 文件头结构
#pragma pack(push, 1)
struct WAVFileHeader {
    char chunkID[4] = {'R', 'I', 'F', 'F'};     // "RIFF"
    uint32_t chunkSize = 0;                      // 文件大小 - 8
    char format[4] = {'W', 'A', 'V', 'E'};      // "WAVE"
};

struct WAVFormatChunk {
    char subchunk1ID[4] = {'f', 'm', 't', ' '};  // "fmt "
    uint32_t subchunk1Size = 16;                 // PCM格式为16
    uint16_t audioFormat = 1;                    // PCM = 1
    uint16_t numChannels = 0;                    // 声道数
    uint32_t sampleRate = 0;                     // 采样率
    uint32_t byteRate = 0;                       // 字节率 = SampleRate * NumChannels * BitsPerSample/8
    uint16_t blockAlign = 0;                     // 块对齐 = NumChannels * BitsPerSample/8
    uint16_t bitsPerSample = 0;                  // 位深度
};

struct WAVDataChunk {
    char subchunk2ID[4] = {'d', 'a', 't', 'a'};  // "data"
    uint32_t subchunk2Size = 0;                  // 数据大小
};
#pragma pack(pop)

// 支持的位深度枚举
enum class BitDepth {
    BIT_8 = 8,
    BIT_16 = 16,
    BIT_24 = 24,
    BIT_32 = 32
};

// WAV 音频类
class WAVAudio {
private:
    WAVFileHeader fileHeader;
    WAVFormatChunk formatChunk;
    WAVDataChunk dataChunk;
    
    // 音频数据存储 - 每个通道一个vector
    std::vector<std::vector<int32_t>> channelData;  // 统一用int32_t存储，方便处理不同位深度
    
    uint32_t sampleRate;
    BitDepth bitDepth;
    uint16_t numChannels;
    
    // 辅助函数
    void updateHeaders();
    uint32_t calculateDataSize() const;
    void writeHeader(std::ofstream& file) const;
    void writeAudioData(std::ofstream& file) const;
    
    // 位深度转换函数
    void writeSample8(std::ofstream& file, int32_t sample) const;
    void writeSample16(std::ofstream& file, int32_t sample) const;
    void writeSample24(std::ofstream& file, int32_t sample) const;
    void writeSample32(std::ofstream& file, int32_t sample) const;
    
    // 范围限制函数
    int32_t clampSample(int32_t sample, BitDepth depth) const;

public:
    WAVAudio();
    WAVAudio(uint32_t sampleRate, BitDepth bitDepth, uint16_t numChannels);
    ~WAVAudio();

    // 设置音频参数
    void setSampleRate(uint32_t rate) { sampleRate = rate; }
    void setBitDepth(BitDepth depth) { bitDepth = depth; }
    void setNumChannels(uint16_t channels);
    
    // 获取音频参数
    uint32_t getSampleRate() const { return sampleRate; }
    BitDepth getBitDepth() const { return bitDepth; }
    uint16_t getNumChannels() const { return numChannels; }
    uint32_t getSampleCount() const;
    
    // 添加音频数据
    bool addChannelData(const std::vector<int>& data, uint16_t channelIndex);
    bool addAllChannelData(const std::vector<std::vector<int>>& allChannelData);
    
    // 清空数据
    void clearData();
    
    // 获取指定通道的数据
    const std::vector<int32_t>& getChannelData(uint16_t channelIndex) const;
    
    // 写入WAV文件
    bool writeWAV(const std::string& filename) const;
    
    // 读取WAV文件
    bool readWAV(const std::string& filename);
    
    // 调试信息
    void printInfo() const;
};

// 便捷函数 - 直接从多个int数组创建WAV文件
bool createWAVFromArrays(const std::vector<std::vector<int>>& channelArrays,
                        uint32_t sampleRate,
                        BitDepth bitDepth,
                        const std::string& filename);

// 便捷函数 - 创建单声道WAV文件
bool createMonoWAV(const std::vector<int>& audioData,
                  uint32_t sampleRate,
                  BitDepth bitDepth,
                  const std::string& filename);

// 便捷函数 - 创建立体声WAV文件
bool createStereoWAV(const std::vector<int>& leftChannel,
                    const std::vector<int>& rightChannel,
                    uint32_t sampleRate,
                    BitDepth bitDepth,
                    const std::string& filename);

#endif // WAV_HANDLER_H
