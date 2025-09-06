#ifndef PACK_H
#define PACK_H

#include <vector>

// External global variables
extern std::vector<int> finalCompressed[2]; // Store final compressed result
extern int frameSize; // Declare frameSize as external variable

// Function declarations
void compress_and_append_frame(int m, int frame_id);
void pack_signal(int m);
void append_frame_to_play_bin(int m, int frame_id);
void write_info_to_play_bin(bool is_gif);
void update_gif_info_framesize();
void initialize_play_bin_for_gif();
void finalize_play_bin();

#endif // PACK_H
