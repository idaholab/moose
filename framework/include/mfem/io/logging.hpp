#pragma once
#include "mfem.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/stopwatch.h"

namespace spdlog::sinks
{

template <typename Mutex>
class MpiSink : public spdlog::sinks::base_sink<Mutex>
{
protected:
  void sink_it_(const spdlog::details::log_msg & msg) override;
  void flush_() override;
};

#include "spdlog/details/null_mutex.h"
#include <mutex>
using MpiSink_mt = MpiSink<std::mutex>;
using MpiSink_st = MpiSink<spdlog::details::null_mutex>;

} // namespace spdlog::sinks

// Global logger
namespace hephaestus
{
extern spdlog::logger logger;
extern int GetGlobalPrintLevel();
}