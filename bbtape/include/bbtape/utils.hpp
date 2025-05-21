#ifndef BBTAPE_UTILS_HPP
#define BBTAPE_UTILS_HPP

#include <filesystem>
#include <vector>
#include <chrono>
#include <string_view>

#include <bbtape/unit.hpp>

namespace bb::utils
{
  namespace fs = std::filesystem;
  namespace ch = std::chrono;

  fs::path
  get_path_from_string(std::string_view path);

  fs::path
  create_tmp_file();

  fs::path
  atomic_create_tmp_file();

  void
  remove_file(const fs::path & path);

  void
  verify_file_path(const std::filesystem::path & path);

  template < typename duration_type >
  requires requires {
    typename duration_type::rep;
    typename duration_type::period;
  }
  struct time_diff
  {
    ch::time_point< ch::high_resolution_clock > start = ch::high_resolution_clock::now();

    duration_type get()
    {
      auto end = ch::high_resolution_clock::now();
      return ch::duration_cast< duration_type >(end - start);
    }
  };

  template< bb::unit_type T >
  bool
  soft_sort_validation(const bb::unit< T > & src);
}

template< bb::unit_type T >
bool
bb::utils::soft_sort_validation(const bb::unit< T > & src)
{
  return std::is_sorted(src.cbegin(), src.cend());
}

#endif
