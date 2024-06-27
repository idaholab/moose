#include "logging.hpp"

namespace spdlog::sinks
{

template <typename Mutex>
void
MpiSink<Mutex>::sink_it_(const spdlog::details::log_msg & msg)
{
  if (mfem::Mpi::WorldRank() == 0)
  {
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    std::cout << fmt::to_string(formatted);
  }
}

template <typename Mutex>
void
MpiSink<Mutex>::flush_()
{
  std::cout << std::flush;
}

}

// Global logger
namespace hephaestus
{
spdlog::logger logger("Hephaestus", std::make_shared<spdlog::sinks::MpiSink_st>());

int
GetGlobalPrintLevel()
{
  int global_print_level;
  if (logger.level() >= SPDLOG_LEVEL_WARN)
  {
    global_print_level = 0;
  }
  else if (logger.level() == SPDLOG_LEVEL_INFO)
  {
    global_print_level = 1;
  }
  else if (logger.level() == SPDLOG_LEVEL_DEBUG)
  {
    global_print_level = 2;
  }
  else
  {
    global_print_level = 3;
  }

  return global_print_level;
}
}