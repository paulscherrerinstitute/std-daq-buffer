
#include <config.hpp>
#include <iostream>
#include <compression.hpp>
#include "ZmqRecvModule.hpp"

using namespace std;

ZmqRecvModule::ZmqRecvModule(
        RingBuffer &ringBuffer,
        const header_map &header_values) :
            ring_buffer_(ring_buffer_),
            header_values_(header_values),
            is_receiving_(false),
            is_writing_(false)
{}

void ZmqRecvModule::start_recv(
        const string& connect_address,
        const uint8_t n_receiving_threads)
{
    if (is_receiving_ == true) {
        stringstream err_msg;

        using namespace date;
        using namespace chrono;
        err_msg << "[" << system_clock::now() << "]";
        err_msg << "[ZmqRecvModule::start_recv]";
        err_msg << " Receivers already running." << endl;

        throw runtime_error(err_msg.str());
    }

    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono;
        cout << "[" << system_clock::now() << "]";
        cout << "[ZmqRecvModule::start]";
        cout << " Starting with parameters:";
        cout << "\tconnect_address: " << connect_address;
        cout << "\tn_receiving_thread: " << (int) n_receiving_threads << endl;
    #endif

    is_receiving_ = true;

    for (uint8_t i_rec=0; i_rec < n_receiving_threads; i_rec++) {
        receiving_threads_.emplace_back(
                &ZmqRecvModule::receive_thread, this, connect_address);
    }
}

void ZmqRecvModule::stop_recv()
{
    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono;
        cout << "[" << system_clock::now() << "]";
        cout << "[ZmqRecvModule::stop_recv]";
        cout << " Stop receiving threads." << endl;
    #endif

    is_receiving_ = false;

    for (auto& recv_thread:receiving_threads_) {
        if (recv_thread.joinable()) {
            recv_thread.join();
        }
    }

    receiving_threads_.clear();
}

void ZmqRecvModule::start_writing()
{
    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono;
        cout << "[" << system_clock::now() << "]";
        cout << "[ZmqRecvModule::start_writing]";
        cout << " Enable writing." << endl;
    #endif

    is_writing_ = true;
}

void ZmqRecvModule::stop_writing()
{
    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono;
        cout << "[" << system_clock::now() << "]";
        cout << "[ZmqRecvModule::stop_writing]";
        cout << " Enable writing." << endl;
    #endif

    is_writing_ = false;
}

void ZmqRecvModule::receive_thread(const string& connect_address)
{
    ZmqReceiver receiver(
            connect_address,
            config::zmq_n_io_threads,
            config::zmq_receive_timeout,
            header_values_);

    receiver.connect();

    while (is_receiving_.load(memory_order_relaxed)) {

        auto frame = receiver.receive();

        // If no message, .first and .second = nullptr
        if (frame.first == nullptr ||
            !is_writing_.load(memory_order_relaxed)) {
            continue;
        }

        auto frame_metadata = frame.first;
        auto frame_data = frame.second;

        #ifdef DEBUG_OUTPUT
            using namespace date;
            using namespace chrono;
            cout << "[" << system_clock::now() << "]";
            cout << "[ZmqRecvModule::receive_thread]";
            cout << " Processing FrameMetadata with frame_index ";
            cout << frame_metadata->frame_index;
            cout << " and frame_shape [" << frame_metadata->frame_shape[0];
            cout << ", " << frame_metadata->frame_shape[1] << "]";
            cout << " and endianness " << frame_metadata->endianness;
            cout << " and type " << frame_metadata->type;
            cout << " and frame_bytes_size ";
            cout << frame_metadata->frame_bytes_size << "." << endl;
        #endif

        char* buffer = ring_buffer_.reserve(frame_metadata);

        // TODO: Add flag to disable compression.
        {
            // TODO: Cache results no to calculate this every time.
            size_t max_buffer_size =
                    compression::get_bitshuffle_max_buffer_size(
                    frame_metadata->frame_bytes_size, 1);

            if (max_buffer_size > ring_buffer_.get_slot_size()) {
                //TODO: Throw error if not large enough.
            }
        }

        auto compressed_size = compression::compress_bitshuffle(
                static_cast<const char*>(frame_data),
                frame_metadata->frame_bytes_size,
                1,
                buffer);

        #ifdef DEBUG_OUTPUT
            using namespace date;
            using namespace chrono;
            cout << "[" << system_clock::now() << "]";
            cout << "[ZmqRecvModule::receive_thread]";
            cout << " Compressed image from ";
            cout << frame_metadata->frame_bytes_size << " bytes to ";
            cout << compressed_size << " bytes." << endl;
        #endif

        frame_metadata->frame_bytes_size = compressed_size;

        ring_buffer_.commit(frame_metadata);
    }

    receiver.disconnect();

    #ifdef DEBUG_OUTPUT
        using namespace date;
        using namespace chrono;
        cout << "[" << system_clock::now() << "]";
        cout << "[ZmqRecvModule::receive_thread]";
        cout << " Receiver thread stopped." << endl;
    #endif
}