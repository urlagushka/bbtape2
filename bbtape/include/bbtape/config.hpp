#ifndef BBTAPE_CONFIG_HPP
#define BBTAPE_CONFIG_HPP

#include <cstddef>
#include <filesystem>

namespace bb
{
  namespace fs = std::filesystem;

  struct delay
  {
    std::size_t on_read;
    std::size_t on_write;
    std::size_t on_roll;
    std::size_t on_offset;
  };

  struct phlimit
  {
    std::size_t ram;
    std::size_t conv;
  };

  struct config
  {
    delay m_delay;
    phlimit m_phlimit;
  };

  config
  read_config_from_file(const fs::path & path);
}

#endif
