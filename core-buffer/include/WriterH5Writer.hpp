#ifndef SFWRITER_HPP
#define SFWRITER_HPP

#include <memory>
#include <string>
#include <H5Cpp.h>
#include "buffer_config.hpp"

struct ImageMetadata
{
    uint64_t pulse_id[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint64_t frame_index[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint32_t daq_rec[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint8_t is_good_frame[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
    uint64_t data_n_bytes[core_buffer::WRITER_DATA_CACHE_N_IMAGES];
};

class WriterH5Writer {

    const size_t n_frames_;
    const size_t n_modules_;
    const size_t image_cache_n_images_;
    size_t current_write_index_;

    H5::H5File file_;

    H5::DataSet image_dataset_;
    H5::DataSet pulse_id_dataset_;
    H5::DataSet frame_index_dataset_;
    H5::DataSet daq_rec_dataset_;
    H5::DataSet is_good_frame_dataset_;


public:
    WriterH5Writer(const std::string& output_file,
                   const size_t n_frames,
                   const size_t n_modules,
                   const size_t image_cache_n_images);
    ~WriterH5Writer();
    void write(const ImageMetadata* metadata, const char* data);
    void close_file();
};


#endif //SFWRITER_HPP
