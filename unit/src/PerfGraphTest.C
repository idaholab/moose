//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "PerfGraph.h"
#include "PerfGuard.h"

TEST(PerfGraphTest, test)
{
  PerfGraph graph;

  //  graph.setActive(false);

  auto a_id = graph.registerSection("a");
  auto b_id = graph.registerSection("b");
  auto c_id = graph.registerSection("c");

  unsigned int a = 0;
  unsigned int b = 0;
  unsigned int c = 0;

  std::vector<double> a_vec(1000000, 2);
  std::vector<double> b_vec(1000000, 3);
  std::vector<double> c_vec(1000000, 2);

  auto start = std::chrono::steady_clock::now();

  for (unsigned int i = 0; i < 1000; i++)
  {
    {
      PerfGuard guard(graph, a_id);
      for (auto & aval : a_vec)
        aval += a++;

      {
        PerfGuard guard(graph, c_id);
        for (auto & cval : c_vec)
          cval += c++;
      }
    }

    {
      PerfGuard guard(graph, b_id);
      for (auto & bval : b_vec)
        bval += b++;

      {
        PerfGuard guard(graph, c_id);
        for (auto & cval : c_vec)
          cval += c++;
      }
    }
  }

  auto end = std::chrono::steady_clock::now();

  auto duration = std::chrono::duration<double, std::milli>(end - start);

  std::cout << "Time: " << duration.count() << std::endl;

  std::cout << a << b << c << std::endl;

  graph.print();
}
