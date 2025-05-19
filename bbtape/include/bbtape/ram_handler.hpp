#ifndef BBTAPE_RAM_HANDLER_HPP
#define BBTAPE_RAM_HANDLER_HPP

#include <memory>
#include <vector>
#include <cstddef>
#include <span>
#include <atomic>
#include <mutex>

#include <bbtape/unit.hpp>

namespace bb
{
  template< unit_type T >
  using unique_ram = std::unique_ptr< std::vector< T > >;

  template< unit_type T >
  using ram_view = std::span< T >;

  template< unit_type T >
  class ram_handler
  {
    public:
      ram_handler() = delete;
      ram_handler(unique_ram< T > ram, std::size_t block_size);

      ram_view< T > take_ram_block();
      void free_ram_block(ram_view< T > block);
      unique_ram< T > pick_ram();

    private:
      std::mutex __mutex;
      unique_ram< T > __ram;
      std::vector< std::atomic_flag > __blocks;
      std::size_t __block_size;
  };

  std::size_t
  balance_ram_block(std::size_t ram_size, std::size_t lhs_size, std::size_t rhs_size);
}

template< bb::unit_type T >
bb::ram_handler< T >::ram_handler(unique_ram< T > ram, std::size_t block_size):
  __mutex(),
  __ram(std::move(ram)),
  __blocks(__ram->size() / block_size),
  __block_size(block_size)
{
  for (auto & flag : __blocks)
  {
    flag.clear();
  }
}

template< bb::unit_type T >
bb::ram_view< T >
bb::ram_handler< T >::take_ram_block()
{
  auto tmp = std::find_if(__blocks.begin(), __blocks.end(), [](std::atomic_flag & flag)
  {
    return !flag.test_and_set();
  });
  if (tmp == __blocks.end())
  {
    throw std::runtime_error("no available ram!");
  }

  std::size_t offset = std::distance(__blocks.begin(), tmp);
  auto start = __ram->data() + offset * __block_size;
  return ram_view< T >{start, start + __block_size};
}

template< bb::unit_type T >
void
bb::ram_handler< T >::free_ram_block(ram_view< T > block)
{
  if (block.data() - __ram->data() < 0)
  {
    throw std::runtime_error("block is not owned by ram!");
  }

  __blocks[(block.data() - __ram->data()) / __block_size].clear();
}

template< bb::unit_type T >
bb::unique_ram< T >
bb::ram_handler< T >::pick_ram()
{
  std::lock_guard< std::mutex > lock(__mutex);
  __blocks.clear();
  __block_size = 0;
  return std::move(__ram);
}

#endif
