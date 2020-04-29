#include "FastQueue.hpp"
#include "gtest/gtest.h"

using namespace core_buffer;

TEST(FastQueue, basic_interaction)
{

    size_t n_slots = 5;
    size_t slot_data_n_bytes = MODULE_N_BYTES * 2;
    FastQueue<DetectorFrame> queue(slot_data_n_bytes, n_slots);
    int slot_id;

    // The queue at the beginning should be empty.
    ASSERT_EQ(queue.read(), -1);
    // Cannot commit a slot until you reserve it.
    ASSERT_THROW(queue.commit(), runtime_error);
    // Cannot release a slot until its ready.
    ASSERT_THROW(queue.release(), runtime_error);

    // Reserve a slot.
    slot_id = queue.reserve();
    ASSERT_NE(slot_id, -1);
    // But you cannot reserve 2 slots at once.
    ASSERT_EQ(queue.reserve(), -1);
    // And cannot read this slot until its committed.
    ASSERT_EQ(queue.read(), -1);

    auto detector_frame = queue.get_metadata_buffer(slot_id);
    char* meta_ptr = (char*) detector_frame;
    char* data_ptr = (char*) queue.get_data_buffer(slot_id);

    queue.commit();

    slot_id = queue.read();
    // Once the slot is committed we should be able to read it.
    ASSERT_NE(slot_id, -1);
    // But you cannot read the next slot until you release the current.
    ASSERT_EQ(queue.read(), -1);
    // The 2 buffers should match the committed slot.
    ASSERT_EQ(meta_ptr, (char*)(queue.get_metadata_buffer(slot_id)));
    ASSERT_EQ(data_ptr, (char*)(queue.get_data_buffer(slot_id)));

    queue.release();
}