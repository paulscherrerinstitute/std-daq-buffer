#include <iostream>
#include <zmq.h>
#include "stream_config.hpp"
#include <chrono>
#include <thread>
#include <StreamSendConfig.hpp>
#include "RamBuffer.hpp"
#include <BufferUtils.hpp>


using namespace std;
using namespace stream_config;
using namespace buffer_config;

int main (int argc, char *argv[])
{
    if (argc != 4) {
        cout << endl;
        cout << "Usage: std_stream_send [detector_json_filename]"
                " [bit_depth] [stream_address]" << endl;
        cout << "\tdetector_json_filename: detector config file path." << endl;
        cout << "\tbit_depth: bit depth of the incoming udp packets." << endl;
        cout << "\tstream_address: address to bind the output stream." << endl;
        cout << endl;

        exit(-1);
    }

    const auto config = StreamSendConfig::from_json_file(string(argv[1]));
    const int bit_depth = atoi(argv[2]);
    const string stream_address = string(argv[3]);

    auto ctx = zmq_ctx_new();
    zmq_ctx_set(ctx, ZMQ_IO_THREADS, STREAM_ZMQ_IO_THREADS);

    auto sender = BufferUtils::bind_socket(
            ctx, config.detector_name, stream_address.c_str());

    auto receiver_assembler = BufferUtils::connect_socket(
            ctx, config.detector_name, "assembler");

    const size_t IMAGE_N_BYTES = config.image_height * config.image_width * bit_depth / 8;
    RamBuffer image_buffer(config.detector_name + "_assembler",
            sizeof(ImageMetadata), IMAGE_N_BYTES,
            1, RAM_BUFFER_N_SLOTS);

    ImageMetadata meta;

    while (true) {

        // receives the assembled image id from the assembler
        zmq_recv(receiver_assembler, &meta, sizeof(meta), 0);
        

        if (meta.id % 10){
            auto* dst_meta = image_buffer.get_slot_meta(meta.id);
            auto* dst_data = image_buffer.get_slot_data(meta.id);
            zmq_send(sender, 
                &meta,
                sizeof(ImageMetadata),  
                ZMQ_SNDMORE | ZMQ_NOBLOCK);

                zmq_send(sender,
                dst_data,
                IMAGE_N_BYTES, ZMQ_NOBLOCK);
        }
    }
}
