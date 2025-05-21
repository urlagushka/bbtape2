#include <gtest/gtest.h>
#include <bbtape/tape_handler.hpp>
#include <filesystem>
#include <thread>
#include <vector>

TEST(tape_handler_test, init) 
{
  bb::config m_config = {{0, 0, 0, 0}, {1, 1}};
  bb::unit< int32_t > data = {1, 2, 3, 4, 5};
  auto tape = std::make_unique< bb::unit< int32_t > >(data.begin(), data.end());
  auto thandler = bb::tape_handler< int32_t >(m_config);
  thandler.setup_tape(std::move(tape));

  EXPECT_EQ(thandler.get_pos(), 0);
  EXPECT_EQ(thandler.size(), 5);
  EXPECT_FALSE(thandler.is_available());
  EXPECT_FALSE(thandler.is_reserved());
}

TEST(tape_handler_test, read_and_write) 
{
  bb::config m_config = {{0, 0, 0, 0}, {1, 1}};
  bb::unit< int32_t > data = {1, 2, 3, 4, 5};
  auto tape = std::make_unique< bb::unit< int32_t > >(data.begin(), data.end());
  auto thandler = bb::tape_handler< int32_t >(m_config);
  thandler.setup_tape(std::move(tape));

  EXPECT_EQ(thandler.read(), 1);
  thandler.write(10);
  EXPECT_EQ(thandler.read(), 10);
}

TEST(tape_handler_test, roll_and_offset) 
{
  bb::config m_config = {{0, 0, 0, 0}, {1, 1}};
  bb::unit< int32_t > data = {1, 2, 3, 4, 5};
  auto tape = std::make_unique< bb::unit< int32_t > >(data.begin(), data.end());
  auto thandler = bb::tape_handler< int32_t >(m_config);
  thandler.setup_tape(std::move(tape));

  thandler.roll(2);
  EXPECT_EQ(thandler.get_pos(), 2);

  thandler.offset(1);
  EXPECT_EQ(thandler.get_pos(), 3);

  thandler.offset(-1);
  EXPECT_EQ(thandler.get_pos(), 2);
}

TEST(tape_handler_test, on_edge) 
{
  bb::config m_config = {{0, 0, 0, 0}, {1, 1}};
  bb::unit< int32_t > data = {1, 2, 3, 4, 5};
  auto tape = std::make_unique< bb::unit< int32_t > >(data.begin(), data.end());
  auto thandler = bb::tape_handler< int32_t >(m_config);
  thandler.setup_tape(std::move(tape));

  EXPECT_THROW(thandler.roll(10), std::runtime_error);
  EXPECT_THROW(thandler.offset(-1), std::runtime_error);
  thandler.roll(thandler.size() - 1);
  EXPECT_THROW(thandler.offset(1), std::runtime_error);
}

TEST(tape_handler_test, offset_if_possible) 
{
  bb::config m_config = {{0, 0, 0, 0}, {1, 1}};
  bb::unit< int32_t > data = {1, 2, 3, 4, 5};
  auto tape = std::make_unique< bb::unit< int32_t > >(data.begin(), data.end());
  auto thandler = bb::tape_handler< int32_t >(m_config);
  thandler.setup_tape(std::move(tape));

  thandler.offset_if_possible(-1);
  EXPECT_EQ(thandler.get_pos(), 0);

  thandler.offset_if_possible(1);
  EXPECT_EQ(thandler.get_pos(), 1);
}

TEST(tape_handler_test, take_and_free) 
{
  bb::config m_config = {{0, 0, 0, 0}, {1, 1}};
  bb::unit< int32_t > data = {1, 2, 3, 4, 5};
  auto tape = std::make_unique< bb::unit< int32_t > >(data.begin(), data.end());
  auto thandler = bb::tape_handler< int32_t >(m_config);

  thandler.take();
  EXPECT_TRUE(thandler.is_reserved());
  EXPECT_THROW(thandler.take(), std::runtime_error);
  thandler.free();
  EXPECT_FALSE(thandler.is_reserved());
}

TEST(tape_handler_test, setup_and_release) 
{
  bb::config m_config = {
    {0, 0, 0, 0},
    {1, 1}
  };
  bb::unit< int32_t > data = {1, 2, 3, 4, 5};
  auto tape = std::make_unique< bb::unit< int32_t > >(data.begin(), data.end());
  auto thandler = bb::tape_handler< int32_t >(m_config);
  thandler.setup_tape(std::move(tape));

  EXPECT_EQ(thandler.size(), 5);

  tape = std::move(thandler.release_tape());
  EXPECT_TRUE(thandler.is_available());
  EXPECT_EQ(tape->size(), 5);
}
