#include <cstddef>
#include <formats.hpp>
#include <chrono>
#include "broker_format.hpp"

#ifndef SF_DAQ_BUFFER_FRAMESTATS_HPP
#define SF_DAQ_BUFFER_FRAMESTATS_HPP


class WriterStats {
    const std::string detector_name_;
    const size_t stats_modulo_;

    uint32_t image_n_bytes_;

    int image_counter_;
    uint64_t total_bytes_;

    uint32_t total_buffer_write_us_;
    uint32_t max_buffer_write_us_;
    std::chrono::time_point<std::chrono::steady_clock> stats_interval_start_;

    void reset_counters();
    void print_stats();

public:
    WriterStats(
            const std::string &detector_name,
            const size_t stats_modulo);
    void setup_run(const StoreStream& meta);
    void start_image_write();
    void end_image_write();
};


#endif //SF_DAQ_BUFFER_FRAMESTATS_HPP
