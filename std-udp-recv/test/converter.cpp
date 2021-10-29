#include <cstdint>
#include <cstring>
#include <iostream>

int main(int argc, char **argv) {
    const int n_packets = 128;
    const int n_pixels_packet = 4096;
    const int n_pixels_frame = n_packets * n_pixels_packet;

    uint16_t *recv_buffer = new uint16_t[n_pixels_packet];

    uint32_t *buffer_0 = new uint32_t[n_pixels_frame];
    uint32_t *buffer_1 = new uint32_t[n_pixels_frame];
    uint32_t *buffer_2 = new uint32_t[n_pixels_frame];

    uint16_t *pede_0 = new uint16_t[n_pixels_frame];
    uint16_t *pede_1 = new uint16_t[n_pixels_frame];
    uint16_t *pede_2 = new uint16_t[n_pixels_frame];

    float *buffer_result = new float[n_pixels_frame];

    float *gain_0 = new float[n_pixels_frame];
    float *gain_1 = new float[n_pixels_frame];
    float *gain_2 = new float[n_pixels_frame];

//    for (int i=0; i<10; i++) {
        for (int n_iter = 0; n_iter < 10000; n_iter++) {
            memset(buffer_0, 0, sizeof(uint32_t) * n_pixels_frame);
            memset(buffer_1, 0, sizeof(uint32_t) * n_pixels_frame);
            memset(buffer_2, 0, sizeof(uint32_t) * n_pixels_frame);

            int pixel_offset = 0;
            for (int i_packet = 0; i_packet < n_packets; i_packet++) {
                for (int i_pixel = 0; i_pixel < n_pixels_packet; i_pixel++) {

                    const unsigned int gain = recv_buffer[i_pixel] >> 14;
                    const uint32_t value = recv_buffer[i_pixel] & 0x3FFF;

                    buffer_0[pixel_offset] += value * (gain & 0x00);
                    buffer_1[pixel_offset] += value * (gain & 0x01);
                    buffer_2[pixel_offset] += value * (gain & 0x10);

                    pixel_offset++;
                }
            }
            for (int i_pixel = 0; i_pixel < n_pixels_frame; i_pixel++) {
                const float val_gain_0 =
                        (buffer_0[i_pixel] - pede_0[i_pixel]) * gain_0[i_pixel];
                const float val_gain_1 =
                        (buffer_1[i_pixel] - pede_1[i_pixel]) * gain_1[i_pixel];
                const float val_gain_2 =
                        (buffer_2[i_pixel] - pede_2[i_pixel]) * gain_2[i_pixel];

                buffer_result[i_pixel] = val_gain_0 + val_gain_1 + val_gain_2;
            }
        }
//    }

    delete [] recv_buffer;
    delete [] buffer_0;
    delete [] buffer_1;
    delete [] buffer_2;
    delete [] buffer_result;
    delete [] gain_0;
    delete [] gain_1;
    delete [] gain_2;

    delete [] pede_0;
    delete [] pede_1;
    delete [] pede_2;

    return 0;
}
