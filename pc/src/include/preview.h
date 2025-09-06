#ifndef PREVIEW_H
#define PREVIEW_H

#include <vector>
#include <utility>
#include <cstdint>
#include "canny.h"

using std::vector;
using std::pair;
using pii = pair<int, int>;

// External global variables
extern grayMatrix_t preview;

// Function declarations
bool connected(pii p1, pii p2);
void color(pii p, uint8_t c);
void Bresenham(pii p1, pii p2);
void preview_signal();

#endif // PREVIEW_H
