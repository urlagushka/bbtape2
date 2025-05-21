#include <gtest/gtest.h>
#include <bbtape/ram_handler.hpp>
#include <vector>

TEST(ram_handler_test, init) 
{
  auto ram = std::make_unique< std::vector< int32_t > >(100);
  bb::ram_handler rhandler(std::move(ram), 10);

  EXPECT_EQ(ram, nullptr);
}

TEST(ram_handler_test, take_and_free) 
{
  auto ram = std::make_unique< std::vector< int32_t > >(100);
  bb::ram_handler rhandler(std::move(ram), 10);

  auto block1 = rhandler.take_ram_block();
  EXPECT_EQ(block1.size(), 10);

  auto block2 = rhandler.take_ram_block();
  EXPECT_EQ(block2.size(), 10);

  EXPECT_NE(block1.data(), block2.data());
  EXPECT_NO_THROW(rhandler.free_ram_block(block1));

  auto block3 = rhandler.take_ram_block();
  EXPECT_EQ(block3.size(), 10);
}

TEST(ram_handler_test, out_of_memory) 
{
  auto ram = std::make_unique< std::vector< int32_t > >(100);
  bb::ram_handler rhandler(std::move(ram), 10);

  for (std::size_t i = 0; i < 10; ++i) 
  {
    rhandler.take_ram_block();
  }

  EXPECT_THROW(rhandler.take_ram_block(), std::runtime_error);
}

TEST(ram_handler_test, free_invalid_block) 
{
  auto ram = std::make_unique< std::vector< int32_t > >(100);
  bb::ram_handler rhandler(std::move(ram), 10);

  std::vector< int32_t > invalid_data(10);
  bb::ram_view< int32_t > invalid_block{invalid_data.data(), invalid_data.size()};

  EXPECT_THROW(rhandler.free_ram_block(invalid_block), std::runtime_error);
}

TEST(ram_handler_test, pick_ram) 
{
  auto ram = std::make_unique< std::vector< int32_t > >(100);
  bb::ram_handler rhandler(std::move(ram), 10);

  ram = std::move(rhandler.pick_ram());
  EXPECT_NE(ram, nullptr);
  EXPECT_THROW(rhandler.take_ram_block(), std::runtime_error);
}
