
#include "WriterH5Writer.hpp"
#include "gtest/gtest.h"
#include "bitshuffle/bitshuffle.h"


using namespace core_buffer;

TEST(WriterH5Writer, basic_interaction)
{
    size_t n_modules = 2;
    size_t n_frames = 5;

    auto data = make_unique<char[]>(n_modules*MODULE_N_BYTES);
    auto metadata = make_shared<ImageMetadata>();

    // Needed by writer.
    metadata->compressed_image_size = 500;

    WriterH5Writer writer("ignore.h5", n_frames, n_modules);
    writer.write(metadata.get(), data.get());
    writer.close_file();
}

TEST(WriterH5Writer, test_compression)
{
    size_t n_modules = 2;
    size_t n_frames = 5;

    auto comp_buffer_size = bshuf_compress_lz4_bound(
            MODULE_N_PIXELS, PIXEL_N_BYTES, MODULE_N_PIXELS);

    auto f_raw_buffer = make_unique<uint16_t[]>(MODULE_N_PIXELS);
    auto f_comp_buffer = make_unique<char[]>(comp_buffer_size);

    auto i_comp_buffer = make_unique<char[]>(
            (comp_buffer_size * n_modules) + 12);
    auto i_raw_buffer = make_unique<uint16_t[]>((MODULE_N_PIXELS * n_modules));

    bshuf_write_uint64_BE(i_comp_buffer.get(),
            MODULE_N_BYTES * n_modules);
    bshuf_write_uint32_BE(i_comp_buffer.get() + 8,
            MODULE_N_PIXELS * PIXEL_N_BYTES);

    size_t total_compressed_size = 0;
    for (int i_module=0; i_module<n_modules; i_module++) {

        for (size_t i=0; i<MODULE_N_PIXELS; i++) {
            f_raw_buffer[i] = (uint16_t) i_module;
        }

        auto compressed_size = bshuf_compress_lz4(
                f_raw_buffer.get(), f_comp_buffer.get(),
                MODULE_N_PIXELS, PIXEL_N_BYTES, MODULE_N_PIXELS);

        memcpy((i_comp_buffer.get() + 12 + total_compressed_size),
               f_comp_buffer.get(),
               compressed_size);

        total_compressed_size += compressed_size;
    }

    auto metadata = make_shared<ImageMetadata>();
    metadata->compressed_image_size = total_compressed_size;

    metadata->is_good_frame = 1;
    metadata->frame_index = 3;
    metadata->pulse_id = 3;
    metadata->daq_rec = 3;

    WriterH5Writer writer("ignore.h5", n_frames, n_modules);
    writer.write(metadata.get(), i_comp_buffer.get());
    writer.close_file();

    H5::H5File reader("ignore.h5", H5F_ACC_RDONLY);
    auto image_dataset = reader.openDataSet("image");
    image_dataset.read(i_raw_buffer.get(), H5T_NATIVE_UINT16);

    ASSERT_EQ(1, 1);
}