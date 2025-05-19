#ifndef BBTAPE_SORT_HPP
#define BBTAPE_SORT_HPP

#include <filesystem>
#include <iostream>
#include <format>
#include <chrono>

#include <bbtape/config.hpp>
#include <bbtape/unit.hpp>
#include <bbtape/tape_handler.hpp>
#include <bbtape/sort_impl.hpp>

namespace bb
{
  namespace fs = std::filesystem;

  struct sort_params
  {
    std::size_t file_amount;
    std::size_t thread_amount;
    std::size_t block_size;
  };

  sort_params
  get_sort_params(std::size_t tape_size, std::size_t ram_size, std::size_t conv_amount);

  template< unit_type T >
  void
  external_merge_sort(config m_config, const fs::path & src, const fs::path & dst, bool is_bench = false);
}

template< bb::unit_type T >
void
bb::external_merge_sort(config m_config, const fs::path & src, const fs::path & dst, bool is_bench)
{
  if (is_bench)
  {
    std::cout << "external_merge_sort start\n";
  }
  auto src_tape = std::make_unique< unit< T > >(read_tape_from_file< T >(src));

  const std::size_t ram_size = m_config.m_phlimit.ram / sizeof(T);
  auto ram = std::make_unique< std::vector< T > >(ram_size);

  sort_params pm = get_sort_params(src_tape->size(), ram_size, m_config.m_phlimit.conv);
  if (pm.thread_amount == 0)
  {
    throw std::runtime_error("conv amount is zero!");
  }

  if (is_bench)
  {
    std::cout << "external_merge_sort params:\n";
    std::cout << std::format("  file_amount: {}\n", pm.file_amount);
    std::cout << std::format("  thread_amount: {}\n", pm.thread_amount);
    std::cout << std::format("  begin block_size: {}\n", pm.block_size);
  }

  std::vector< shared_tape_handler< T > > ths;
  for (std::size_t i = 0; i < m_config.m_phlimit.conv; ++i)
  {
    ths.push_back(std::make_shared< tape_handler< T > >(m_config));
  }

  if (is_bench)
  {
    std::cout << "split_src_unit start\n";
  }
  auto start = std::chrono::high_resolution_clock::now();
  auto files_tape_ram = split_src_unit< T >(std::move(src_tape), ths[0], pm.file_amount, std::move(ram));
  file_handler tmp_files = std::move(std::get< 0 >(files_tape_ram));
  src_tape = std::move(std::get< 1 >(files_tape_ram));
  ram = std::move(std::get< 2 >(files_tape_ram));
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  if (is_bench)
  {
    std::cout << std::format("time: {}ms\n", duration.count());
  }

  if (is_bench)
  {
    std::cout << "strategy start\n";
  }
  start = std::chrono::high_resolution_clock::now();
  std::size_t block_size = pm.block_size;
  std::size_t thread_amount = pm.thread_amount;
  while (tmp_files.size() > 1)
  {
    auto merge = strategy< T >(tmp_files, ths, std::move(ram), block_size, thread_amount);
    tmp_files = std::move(std::get< 0 >(merge));
    ram = std::move(std::get< 1 >(merge));

    thread_amount = std::min(thread_amount, tmp_files.size() / 2);
    block_size = ram_size / thread_amount;
    if (is_bench)
    {
      std::cout << std::format("new block_size: {}\n", block_size);
    }
  }

  auto tape = read_tape_from_file< T >(tmp_files[0]);
  write_tape_to_file(dst, tape);
  end = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  if (is_bench)
  {
    std::cout << std::format("time: {}ms\n", duration.count());
    std::cout << "sort validation\n";
  }

  for (std::size_t i = 0; i < 1000; ++i)
  {
    if (i != tape[i])
    {
      std::cout << "BAD SORT\n";
      std::cout << "i: " << i << " tape[i]: " << tape[i] << "\n";
    }
  }

  if (is_bench)
  {
    std::cout << "done\n";
  }
}

#endif
