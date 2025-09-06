#include "preview.h"
#include "constructor.h"
#include "canny.h"
#include "../drivers/bmp_handler.h"
#include <bits/stdc++.h>

grayMatrix_t preview;

bool connected(pii p1, pii p2) {
    int dx = p1.first - p2.first;
    int dy = p1.second - p2.second;
    dx = std::abs(dx);
    dy = std::abs(dy);
    return std::max(dx, dy) <= 1;
}

void color(pii p, uint8_t c) {
    int x = p.first, y = p.second;
    int height = preview.size();
    int width = preview[0].size();
    
    // 边界检查
    if (x >= 0 && x < height && y >= 0 && y < width) {
        preview[x][y] = std::max(preview[x][y], c);
    }
}

void Bresenham(pii p1, pii p2) {
    int x0 = p1.first, y0 = p1.second;
    int x1 = p2.first, y1 = p2.second;
    
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    
    int sx = x0 < x1 ? 1 : -1;  // x方向步长
    int sy = y0 < y1 ? 1 : -1;  // y方向步长
    
    int err = dx - dy;  // 误差项
    
    int x = x0, y = y0;
    
    while (true) {
        // 对当前点进行染色
        color(std::make_pair(x, y), 127);
        
        // 如果到达终点，退出循环
        if (x == x1 && y == y1) {
            break;
        }
        
        int e2 = 2 * err;
        
        // 决定下一步移动方向
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

void preview_signal() {
    int height = grayMatrix.size();
    int width = grayMatrix[0].size();
    preview = grayMatrix_t(height, vector<uint8_t>(width, 0));

    color(signalXY[0], 255);
    for (int i = 1; i < signalXY.size(); i++) {
        if (!connected(signalXY[i - 1], signalXY[i])) {
            Bresenham(signalXY[i - 1], signalXY[i]);
        }
        color(signalXY[i], 255);
    }
    
    // 保存预览图像
    BMPImage previewImg;
    previewImg.fromGrayscaleMatrix(preview);
    previewImg.writeBMP("preview.bmp");
    std::cout << "Preview signal saved as preview.bmp" << std::endl;
}
