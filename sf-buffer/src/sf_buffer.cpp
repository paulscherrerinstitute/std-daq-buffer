#include <iostream>
#include <stdexcept>
#include <RingBuffer.hpp>
#include <UdpRecvModule.hpp>
#include <FastH5Writer.hpp>

#include "buffer_config.hpp"
#include "jungfrau.hpp"


using namespace std;
using namespace core_buffer;


int main (int argc, char *argv[]) {
    if (argc != 4) {
        cout << endl;
        cout << "Usage: sf_buffer [device_name] [udp_port] [root_folder]";
        cout << endl;
        cout << "\tdevice_name: Name to write to disk.";
        cout << "\tudp_port: UDP port to connect to." << endl;
        cout << "\troot_folder: FS root folder." << endl;
        cout << endl;

        exit(-1);
    }

    string device_name = string(argv[1]);
    int udp_port = atoi(argv[2]);
    string root_folder = string(argv[3]);

    RingBuffer<UdpFrameMetadata> ring_buffer(BUFFER_RB_SIZE);

    UdpRecvModule udp_module(ring_buffer);
    udp_module.start_recv(udp_port, JUNGFRAU_DATA_BYTES_PER_FRAME);

    uint64_t n_stat_out(0);
    uint64_t n_frames_with_missing_packets = 0;
    uint64_t n_missed_frames = 0;
    uint64_t last_pulse_id = 0;

    FastH5Writer writer(
            core_buffer::FILE_MOD, MODULE_Y_SIZE, MODULE_X_SIZE,
            device_name, root_folder);

    writer.add_scalar_metadata<uint64_t>("pulse_id");
    writer.add_scalar_metadata<uint64_t>("frame_id");
    writer.add_scalar_metadata<uint32_t>("daq_rec");
    writer.add_scalar_metadata<uint16_t>("received_packets");

    while (true) {
        auto data = ring_buffer.read();

        if (data.first == nullptr) {
            this_thread::sleep_for(chrono::milliseconds(10));
            continue;
        }

        auto pulse_id = data.first->pulse_id;
        writer.set_pulse_id(pulse_id);

        writer.write_data(data.second);

        // TODO: Combine all this into 1 struct.

        writer.write_scalar_metadata<uint64_t>(
                "pulse_id", &(data.first->pulse_id));

        writer.write_scalar_metadata<uint64_t>(
                "frame_id",
                &(data.first->frame_index));

        writer.write_scalar_metadata<uint32_t>(
                "daq_rec",
                &(data.first->daq_rec));

        writer.write_scalar_metadata<uint16_t>(
                "received_packets",
                &(data.first->n_recv_packets));

        ring_buffer.release(data.first->buffer_slot_index);

        // TODO: Make real statistics, please.
        n_stat_out++;

        if (data.first->n_recv_packets < JUNGFRAU_N_PACKETS_PER_FRAME) {
            n_frames_with_missing_packets +=
                    JUNGFRAU_N_PACKETS_PER_FRAME - data.first->n_recv_packets;
        }

        if (last_pulse_id>0) {
            n_missed_frames += (pulse_id - last_pulse_id) - 1;
        }
        last_pulse_id = pulse_id;

        if (n_stat_out == 500) {
            cout << "device_name " << device_name;
            cout << " pulse_id " << pulse_id;
            cout << " n_missed_frames " << n_missed_frames;
            cout << " f_with_miss_p " << n_frames_with_missing_packets;
            cout << endl;


            n_stat_out = 0;
            n_frames_with_missing_packets = 0;
            n_missed_frames = 0;
        }
    }
}
