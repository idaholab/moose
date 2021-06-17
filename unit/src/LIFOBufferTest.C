//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "LIFOBuffer.h"

#include <algorithm> // std::set_symmetric_difference

TEST(LIFOBuffer, test)
{
  MooseUtils::LIFOBuffer<int> buffer(4);

  EXPECT_EQ(buffer.size(), 0);
  EXPECT_EQ(buffer.capacity(), 4);

  buffer.push_back(0);
  buffer.push_back(1);
  buffer.push_back(2);
  buffer.push_back(3);

  EXPECT_EQ(buffer.dataBeginPos(), 0);
  EXPECT_EQ(buffer.dataEndPos(), 4);

  auto & data = buffer.data();

  EXPECT_EQ(data[0], 0);
  EXPECT_EQ(data[1], 1);
  EXPECT_EQ(data[2], 2);
  EXPECT_EQ(data[3], 3);

  buffer.erase(2);

  EXPECT_EQ(*buffer.begin(), 0);
  EXPECT_EQ(*(buffer.end() - 1), 1);

  EXPECT_EQ(buffer.dataBeginPos(), 0);
  EXPECT_EQ(buffer.dataEndPos(), 2);

  buffer.erase(2);

  EXPECT_EQ(buffer.dataBeginPos(), 0);
  EXPECT_EQ(buffer.dataEndPos(), 0);

  buffer.push_back(0);
  buffer.push_back(1);
  buffer.push_back(2);
  buffer.push_back(3);

  EXPECT_EQ(buffer.dataBeginPos(), 0);
  EXPECT_EQ(buffer.dataEndPos(), 4);

  buffer.push_back(4);

  EXPECT_EQ(buffer.dataBeginPos(), 0);
  EXPECT_EQ(buffer.dataEndPos(), 5);

  EXPECT_EQ(buffer.size(), 5);
  EXPECT_EQ(buffer.capacity(), 10);

  buffer.erase(2);

  EXPECT_EQ(buffer.dataBeginPos(), 0);
  EXPECT_EQ(buffer.dataEndPos(), 3);

  for (unsigned int i = 3; i < 10; i++)
    buffer.push_back(i);

  EXPECT_EQ(buffer.dataBeginPos(), 0);
  EXPECT_EQ(buffer.dataEndPos(), 10);

  buffer.push_back(10);

  EXPECT_EQ(buffer.dataBeginPos(), 0);
  EXPECT_EQ(buffer.dataEndPos(), 11);
  EXPECT_EQ(buffer.size(), 11);
  EXPECT_EQ(buffer.capacity(), 22);

  for (unsigned int i = 0; i < 11; i++)
    EXPECT_EQ(buffer[i], i);

  {
    unsigned int i = 0;

    for (const auto & val : buffer)
    {
      EXPECT_EQ(val, i);
      i++;
    }
  }

  buffer.erase(3);

  EXPECT_EQ(buffer.dataBeginPos(), 0);
  EXPECT_EQ(buffer.dataEndPos(), 8);
  EXPECT_EQ(buffer.size(), 8);
  EXPECT_EQ(buffer.capacity(), 22);

  EXPECT_EQ(buffer[2], 2);

  std::vector<int> dummy = {1, 2, 3, 4};

  buffer.swap(dummy);

  EXPECT_EQ(buffer.dataBeginPos(), 0);
  EXPECT_EQ(buffer.dataEndPos(), 4);
  EXPECT_EQ(buffer.size(), 4);
  EXPECT_EQ(buffer.capacity(), 4);

  buffer.erase(2);
  buffer.push_back(5);

  EXPECT_EQ(buffer.dataBeginPos(), 0);
  EXPECT_EQ(buffer.dataEndPos(), 3);
  EXPECT_EQ(buffer.size(), 3);
  EXPECT_EQ(buffer.capacity(), 4);

  buffer.erase(2);
  buffer.append({6, 7, 8, 9, 10});

  EXPECT_EQ(buffer.dataBeginPos(), 0);
  EXPECT_EQ(buffer.dataEndPos(), 6);
  EXPECT_EQ(buffer.size(), 6);
  EXPECT_EQ(buffer.capacity(), 12);

  EXPECT_EQ(buffer.empty(), false);

  buffer.clear();

  EXPECT_EQ(buffer.empty(), true);

  buffer.setCapacity(21);
  for (unsigned int i = 0; i < 21; i++)
    buffer.push_back(i);

  const unsigned int chunk_size = 5;
  for (unsigned int i = 0; i < 5; i++)
  {
    const auto begin = buffer.beginChunk(chunk_size);
    const auto end = buffer.endChunk(chunk_size);

    if (i < 4)
      EXPECT_EQ(begin, data.end() - chunk_size * (i + 1));
    else
      EXPECT_EQ(begin, data.begin());
    if (i < 4)
      EXPECT_EQ(end, data.end() - chunk_size * i);
    else
      EXPECT_EQ(end, data.begin() + 1);

    buffer.eraseChunk(chunk_size);
    if (i < 4)
      EXPECT_EQ(buffer.size(), 21 - chunk_size * (i + 1));
    else
      EXPECT_EQ(buffer.size(), 0);
  }

  EXPECT_EQ(buffer.empty(), true);
}
