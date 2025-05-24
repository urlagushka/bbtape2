#ifndef BBTAPE_SORT_HPP
#define BBTAPE_SORT_HPP

#include <filesystem>
#include <istream>
#include <format>
#include <chrono>

#include <bbtape/config.hpp>
#include <bbtape/unit.hpp>
#include <bbtape/tape_handler.hpp>
#include <bbtape/sort_impl.hpp>

namespace
{
  struct sort_params
  {
    std::size_t file_amount;
    std::size_t thread_amount;
    std::size_t block_size;
  };

  sort_params
  get_sort_params(std::size_t unit_size, std::size_t ram_size, std::size_t conv_amount)
  {
    std::size_t file_amount = unit_size / ram_size;
    if (unit_size % ram_size != 0)
    {
      ++file_amount;
    }
    if (file_amount % 2 != 0)
    {
      ++file_amount;
    }

    std::size_t thread_amount = std::min(conv_amount, file_amount / 2);
    std::size_t block_size = ram_size / thread_amount;

    return {file_amount, thread_amount, block_size};
  }
}

namespace bb
{
  namespace fs = std::filesystem;
  using optional_out = std::optional< std::reference_wrapper< std::ostream > >;

  template< unit_type T >
  void
  external_merge_sort(config m_config, const fs::path & src, const fs::path & dst, optional_out out = std::nullopt);
}

template< bb::unit_type T >
void
bb::external_merge_sort(config m_config, const fs::path & src, const fs::path & dst, optional_out out)
{
  if (out.has_value())
  {
    out->get() << "EXTERNAL_MERGE_SORT\n";
  }

  auto src_tape = std::make_unique< unit< T > >(read_tape_from_file< T >(src));
  const std::size_t ram_size = m_config.m_phlimit.ram / sizeof(T);
  auto ram = std::make_unique< std::vector< T > >(ram_size);
  sort_params pm = get_sort_params(src_tape->size(), ram_size, m_config.m_phlimit.conv);
  if (pm.thread_amount == 0)
  {
    throw std::runtime_error("conv amount is zero!");
  }

  if (out.has_value())
  {
    out->get() << std::format("> file_amount: {}\n", pm.file_amount);
    out->get() << std::format("> thread_amount: {}\n", pm.thread_amount);
    out->get() << std::format("> begin block_size: {}\n", pm.block_size);
  }

  std::vector< shared_tape_handler< T > > ths;
  for (std::size_t i = 0; i < m_config.m_phlimit.conv; ++i)
  {
    ths.push_back(std::make_shared< tape_handler< T > >(m_config));
  }

  if (out.has_value())
  {
    out->get() << "split_src_unit start\n";
  }

  utils::time_diff< std::chrono::milliseconds > split_time;
  auto files_tape_ram = split_src_unit< T >(std::move(src_tape), ths[0], pm.file_amount, std::move(ram));
  file_handler tmp_files = std::move(std::get< 0 >(files_tape_ram));
  src_tape = std::move(std::get< 1 >(files_tape_ram));
  ram = std::move(std::get< 2 >(files_tape_ram));

  if (out.has_value())
  {
    out->get() << std::format("time: {}ms\n", split_time.get().count());
    out->get() << "strategy start\n";
  }

  utils::time_diff< std::chrono::milliseconds > strategy_time;
  std::size_t block_size = pm.block_size;
  std::size_t thread_amount = pm.thread_amount;
  while (tmp_files.size() > 1)
  {
    auto merge = strategy< T >(tmp_files, ths, std::move(ram), block_size, thread_amount);
    tmp_files = std::move(std::get< 0 >(merge));
    ram = std::move(std::get< 1 >(merge));

    thread_amount = std::min(thread_amount, tmp_files.size() / 2);
    block_size = (thread_amount == 0) ? 0 : ram_size / thread_amount;
  }
  auto tape = read_tape_from_file< T >(tmp_files[0]);
  write_tape_to_file(dst, tape);

  if (out.has_value())
  {
    out->get() << std::format("time: {}ms\n", strategy_time.get().count());

    bool result = utils::soft_sort_validation(tape);
    if (result)
    {
      out->get() << std::format("soft_sort_validation: \033[32msuccess\033[0m\n");
    }
    else
    {
      out->get() << std::format("soft_sort_validation: \033[31mfail\033[0m\n");
    }
  }
}

#endif
