# Oscillo-Show

基于 STM32H750VBT6 单片机的简易示波器动画项目。通过 PC 端处理 .bmp 或 .gif 文件，计算生成 play.bin 二进制文件，使用 SD 卡输入给单片机，通过两个片上 DAC 方波控制 X，Y 信号绘制图案。

对比声卡方案，专职 DAC 的外设可以做到更高的采样率。一般声卡能打到 48kHz左右的量级，而本项目中单片机的工作频率为 480MHz，DAC 采样率设置为 3MHz，可以做到更密集的采样点数和较高的帧率。

项目中也同步实现了 play.wav 双通道音频文件的生成，但是没有追加帧率控制的逻辑，所以暂时无法使用。

部分公式化的代码借助了 AI 生成。

## 用法

`pc/` 下为 PC 端文件，使用 makefile 可以直接生成可执行文件。

- `make` 命令：编译；
- `make run`：编译并运行；
- `make clean`：去除中间文件。

执行过程中，中间和结果文件存放在 `D:/OscilloProj/frames` 和   `D:/OscilloProj/SDFiles` 下。`frames/` 存放 gif 文件逐帧分解的结果，`SDFiles` 存放打包好的结果 `play.bin`。

对于 bmp 文件：

```txt
Entry source file (bmp or gif): samples/natsu2.bmp // 原始文件路径
Entry frame size: 40000	// 点数密度，不要超过 60000
Entry high and low threshold (0.0 - 1.0): 0.02 0.01 // Canny 边缘检测阈值，可以随便试一试
image height: 1133
image width: 1170
Result canny.bmp saved.
Canny edge detection completed. Check canny.bmp file.
Continue with current result? (y/n): y // 可以在此处检查边缘检测结果 canny.bmp，如果输入 n，可以中途对 canny.bmp 更改
points : 21454
edges : 4164
edges : 4164
signal length: 51623
Preview signal saved as preview.bmp
Added BMP info data (FPS=0, framecount=1, framesize=40000) to play.bin
Appended frame 0 data (160000 bytes) to play.bin
SD file saved to: D:/OscilloProj/SDfiles/play.bin (size: 160006 bytes) // 最终得到的文件
WAV file saved successfully: play.wav // 暂时还没法用的 play.wav
```

对于 gif 文件：

```txt
Entry source file (bmp or gif): samples/arona.gif
Entry frame size: 30000
Entry high and low threshold (0.0 - 1.0): 0.2 0.02
Already converted gif to bmp frames? (y/n): n // 如果之前跑过一样的图可以写 y 不用重复逐帧分解
Extracted 67 frames to D:/OscilloProj/frames/
Frame rate: 20.00 fps
Total frames: 67
Keep wav file? (y/n): n
Updated framesize=30000 in info.txt
Added info.txt content (6 bytes) to play.bin
Processing D:/OscilloProj/frames/frame_0000.bmp (frame 0) // 每一帧的提示信息（这个过程可能很慢，和文件复杂程度有关系）
image height: 998
image width: 889
Result canny.bmp saved.
points : 24393
edges : 3257
edges : 3257
signal length: 59828
Appended frame 0 data (120000 bytes) to play.bin
Processing D:/OscilloProj/frames/frame_0001.bmp (frame 1)
image height: 998
image width: 889
Result canny.bmp saved.
points : 23564
edges : 3202
edges : 3202
signal length: 55209
Appended frame 1 data (120000 bytes) to play.bin
// ...
Processing D:/OscilloProj/frames/frame_0066.bmp (frame 66)
image height: 998
image width: 889
Result canny.bmp saved.
points : 22365
edges : 3082
edges : 3082
signal length: 52189
Appended frame 66 data (120000 bytes) to play.bin
Total compressed signal length: 2010000
WAV file saved successfully: play.wav
SD file saved to: D:/OscilloProj/SDfiles/play.bin (size: 8040006 bytes)
```

然后用 SD 卡（FAT32），把 play.bin 存到里面去。我使用的是 SDHC U1/Class 10 32GB，其他的没有测试，可能因为读取过快没法正常读入。

接入单片机后，烧录 `STM32H750VBT6/` 下的代码，具体接线参考代码。将 DAC 两个引脚接上示波器探头，但是要 X、Y 互换并且 Y 反向（偷懒忘记把这个搞正了）。

## 效果

![](natsu.jpg)

Bad Apple 参见 `bad apple.mp4`。

## 细节

更多项目实现上的细节参见我的 blog [示波器动画初体验](https://www.cnblogs.com/Lice-wx/p/19055301/oscilloscope-animation)。