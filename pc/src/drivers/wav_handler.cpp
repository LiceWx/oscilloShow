#include "wav_handler.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>

// 构造函数
WAVAudio::WAVAudio() : sampleRate(44100), bitDepth(BitDepth::BIT_16), numChannels(1) {
    setNumChannels(1);
}

WAVAudio::WAVAudio(uint32_t sampleRate, BitDepth bitDepth, uint16_t numChannels) 
    : sampleRate(sampleRate), bitDepth(bitDepth), numChannels(numChannels) {
    setNumChannels(numChannels);
}

WAVAudio::~WAVAudio() {
}

// 设置通道数
void WAVAudio::setNumChannels(uint16_t channels) {
    if (channels == 0) {
        channels = 1;
    }
    numChannels = channels;
    channelData.resize(channels);
}

// 获取采样点数量
uint32_t WAVAudio::getSampleCount() const {
    if (channelData.empty()) {
        return 0;
    }
    return channelData[0].size();
}

// 添加单个通道数据
bool WAVAudio::addChannelData(const std::vector<int>& data, uint16_t channelIndex) {
    if (channelIndex >= numChannels) {
        std::cerr << "Error: Channel index " << channelIndex << " out of range (total channels: " << numChannels << ")" << std::endl;
        return false;
    }
    
    channelData[channelIndex].clear();
    channelData[channelIndex].reserve(data.size());
    
    for (int sample : data) {
        channelData[channelIndex].push_back(clampSample(sample, bitDepth));
    }
    
    return true;
}

// 添加所有通道数据
bool WAVAudio::addAllChannelData(const std::vector<std::vector<int>>& allChannelData) {
    if (allChannelData.size() != numChannels) {
        std::cerr << "Error: Input channel count " << allChannelData.size() 
                  << " does not match set channel count " << numChannels << std::endl;
        return false;
    }
    
    // 检查所有通道的采样点数量是否一致
    if (!allChannelData.empty()) {
        size_t expectedSize = allChannelData[0].size();
        for (size_t i = 1; i < allChannelData.size(); i++) {
            if (allChannelData[i].size() != expectedSize) {
                std::cerr << "Error: Channel " << i << " sample count (" << allChannelData[i].size()
                          << ") does not match channel 0 sample count (" << expectedSize << ")" << std::endl;
                return false;
            }
        }
    }
    
    for (uint16_t i = 0; i < numChannels; i++) {
        if (!addChannelData(allChannelData[i], i)) {
            return false;
        }
    }
    
    return true;
}

// 清空数据
void WAVAudio::clearData() {
    for (auto& channel : channelData) {
        channel.clear();
    }
}

// 获取指定通道的数据
const std::vector<int32_t>& WAVAudio::getChannelData(uint16_t channelIndex) const {
    static std::vector<int32_t> empty;
    if (channelIndex >= numChannels) {
        std::cerr << "Error: Channel index out of range" << std::endl;
        return empty;
    }
    return channelData[channelIndex];
}

// 范围限制函数
int32_t WAVAudio::clampSample(int32_t sample, BitDepth depth) const {
    switch (depth) {
        case BitDepth::BIT_8:
            return std::max(0, std::min(255, sample));
        case BitDepth::BIT_16:
            return std::max(-32768, std::min(32767, sample));
        case BitDepth::BIT_24:
            return std::max(-8388608, std::min(8388607, sample));
        case BitDepth::BIT_32:
            return sample; // int32_t 的完整范围
        default:
            return sample;
    }
}

// 更新文件头
void WAVAudio::updateHeaders() {
    uint32_t bytesPerSample = static_cast<uint32_t>(bitDepth) / 8;
    uint32_t samplesCount = getSampleCount();
    
    // 更新格式块
    formatChunk.numChannels = numChannels;
    formatChunk.sampleRate = sampleRate;
    formatChunk.bitsPerSample = static_cast<uint16_t>(bitDepth);
    formatChunk.blockAlign = numChannels * bytesPerSample;
    formatChunk.byteRate = sampleRate * numChannels * bytesPerSample;
    
    // 更新数据块
    dataChunk.subchunk2Size = samplesCount * numChannels * bytesPerSample;
    
    // 更新文件头
    fileHeader.chunkSize = 36 + dataChunk.subchunk2Size;
}

// 计算数据大小
uint32_t WAVAudio::calculateDataSize() const {
    uint32_t bytesPerSample = static_cast<uint32_t>(bitDepth) / 8;
    return getSampleCount() * numChannels * bytesPerSample;
}

// 写入不同位深度的采样
void WAVAudio::writeSample8(std::ofstream& file, int32_t sample) const {
    uint8_t value = static_cast<uint8_t>(sample);
    file.write(reinterpret_cast<const char*>(&value), 1);
}

void WAVAudio::writeSample16(std::ofstream& file, int32_t sample) const {
    int16_t value = static_cast<int16_t>(sample);
    file.write(reinterpret_cast<const char*>(&value), 2);
}

void WAVAudio::writeSample24(std::ofstream& file, int32_t sample) const {
    // 24位数据以小端序存储，分3个字节写入
    uint8_t byte1 = sample & 0xFF;
    uint8_t byte2 = (sample >> 8) & 0xFF;
    uint8_t byte3 = (sample >> 16) & 0xFF;
    file.write(reinterpret_cast<const char*>(&byte1), 1);
    file.write(reinterpret_cast<const char*>(&byte2), 1);
    file.write(reinterpret_cast<const char*>(&byte3), 1);
}

void WAVAudio::writeSample32(std::ofstream& file, int32_t sample) const {
    file.write(reinterpret_cast<const char*>(&sample), 4);
}

// 写入文件头
void WAVAudio::writeHeader(std::ofstream& file) const {
    file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<const char*>(&formatChunk), sizeof(formatChunk));
    file.write(reinterpret_cast<const char*>(&dataChunk), sizeof(dataChunk));
}

// 写入音频数据
void WAVAudio::writeAudioData(std::ofstream& file) const {
    uint32_t samplesCount = getSampleCount();
    
    // 交错写入多声道数据
    for (uint32_t i = 0; i < samplesCount; i++) {
        for (uint16_t channel = 0; channel < numChannels; channel++) {
            int32_t sample = (i < channelData[channel].size()) ? channelData[channel][i] : 0;
            
            switch (bitDepth) {
                case BitDepth::BIT_8:
                    writeSample8(file, sample);
                    break;
                case BitDepth::BIT_16:
                    writeSample16(file, sample);
                    break;
                case BitDepth::BIT_24:
                    writeSample24(file, sample);
                    break;
                case BitDepth::BIT_32:
                    writeSample32(file, sample);
                    break;
            }
        }
    }
}

// 写入WAV文件
bool WAVAudio::writeWAV(const std::string& filename) const {
    if (channelData.empty() || getSampleCount() == 0) {
        std::cerr << "Error: No audio data to write" << std::endl;
        return false;
    }
    
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create file " << filename << std::endl;
        return false;
    }
    
    // 需要临时修改对象以更新头部信息
    const_cast<WAVAudio*>(this)->updateHeaders();
    
    writeHeader(file);
    writeAudioData(file);
    
    file.close();
    
    std::cout << "WAV file saved successfully: " << filename << std::endl;
    return true;
}

// 读取WAV文件（基础实现）
bool WAVAudio::readWAV(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    // 读取文件头
    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    file.read(reinterpret_cast<char*>(&formatChunk), sizeof(formatChunk));
    file.read(reinterpret_cast<char*>(&dataChunk), sizeof(dataChunk));
    
    // 验证文件格式
    if (std::memcmp(fileHeader.chunkID, "RIFF", 4) != 0 ||
        std::memcmp(fileHeader.format, "WAVE", 4) != 0 ||
        std::memcmp(formatChunk.subchunk1ID, "fmt ", 4) != 0 ||
        std::memcmp(dataChunk.subchunk2ID, "data", 4) != 0) {
        std::cerr << "Error: Invalid WAV file format" << std::endl;
        return false;
    }
    
    // 设置音频参数
    sampleRate = formatChunk.sampleRate;
    numChannels = formatChunk.numChannels;
    bitDepth = static_cast<BitDepth>(formatChunk.bitsPerSample);
    
    setNumChannels(numChannels);
    
    // 读取音频数据（这里只实现基础读取，可以根据需要扩展）
    std::cout << "WAV file read successfully: " << filename << std::endl;
    printInfo();
    
    file.close();
    return true;
}

// 打印音频信息
void WAVAudio::printInfo() const {
    std::cout << "=== WAV Audio Info ===" << std::endl;
    std::cout << "Sample Rate: " << sampleRate << " Hz" << std::endl;
    std::cout << "Bit Depth: " << static_cast<int>(bitDepth) << " bit" << std::endl;
    std::cout << "Channels: " << numChannels << std::endl;
    std::cout << "Sample Count: " << getSampleCount() << std::endl;
    std::cout << "Duration: " << (getSampleCount() / static_cast<float>(sampleRate)) << " seconds" << std::endl;
    std::cout << "Data Size: " << calculateDataSize() << " bytes" << std::endl;
    std::cout << "===================" << std::endl;
}

// 便捷函数实现

bool createWAVFromArrays(const std::vector<std::vector<int>>& channelArrays,
                        uint32_t sampleRate,
                        BitDepth bitDepth,
                        const std::string& filename) {
    if (channelArrays.empty()) {
        std::cerr << "Error: No input data" << std::endl;
        return false;
    }
    
    WAVAudio wav(sampleRate, bitDepth, channelArrays.size());
    
    if (!wav.addAllChannelData(channelArrays)) {
        return false;
    }
    
    return wav.writeWAV(filename);
}

bool createMonoWAV(const std::vector<int>& audioData,
                  uint32_t sampleRate,
                  BitDepth bitDepth,
                  const std::string& filename) {
    std::vector<std::vector<int>> channelData = {audioData};
    return createWAVFromArrays(channelData, sampleRate, bitDepth, filename);
}

bool createStereoWAV(const std::vector<int>& leftChannel,
                    const std::vector<int>& rightChannel,
                    uint32_t sampleRate,
                    BitDepth bitDepth,
                    const std::string& filename) {
    std::vector<std::vector<int>> channelData = {leftChannel, rightChannel};
    return createWAVFromArrays(channelData, sampleRate, bitDepth, filename);
}
