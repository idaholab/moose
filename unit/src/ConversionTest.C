//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include <vector>
#include <set>

// Moose includes
#include "Conversion.h"
#include "ExecFlagEnum.h"

TEST(Conversion, stringify)
{
  EXPECT_EQ(Moose::stringify("foo"), "foo");
  EXPECT_EQ(Moose::stringify(std::string("foo")), "foo");
  EXPECT_EQ(Moose::stringify(int(123)), "123");
  EXPECT_EQ(Moose::stringify(Real(1.23456)), "1.23456");
  EXPECT_EQ(Moose::stringify(libMesh::pi), "3.14159");

  EXPECT_EQ(Moose::stringify(std::vector<Real>({4.567, 2.397, 3.2734}), ","), "4.567,2.397,3.2734");
  EXPECT_EQ(Moose::stringify(std::set<Real>({4.567, 2.397, 3.2734}), ","), "2.397,3.2734,4.567");

  EXPECT_EQ(Moose::stringify(std::vector<std::vector<Real>>(
                {{1.1, 2.1, 3.1}, {3.2, 2.2, 1.2}, {1.3, 2.3, 3.3}})),
            "1.1, 2.1, 3.1, 3.2, 2.2, 1.2, 1.3, 2.3, 3.3");
  EXPECT_EQ(Moose::stringify(
                std::vector<std::set<Real>>({{1.1, 2.1, 3.1}, {3.2, 2.2, 1.2}, {1.3, 2.3, 3.3}})),
            "1.1, 2.1, 3.1, 1.2, 2.2, 3.2, 1.3, 2.3, 3.3");
  EXPECT_EQ(Moose::stringify(
                std::map<std::string, int>({{"nmax", 2345}, {"size", 17}, {"allocated", 23}}), ","),
            "allocated:23,nmax:2345,size:17");
  EXPECT_EQ(Moose::stringify(std::vector<std::vector<std::string>>(
                                 {{"Streets", "full", "of", "water"}, {"Please", "advise"}}),
                             "--STOP--"),
            "Streets, full, of, water--STOP--Please, advise");

  EXPECT_EQ(
      Moose::stringify(
          std::vector<Moose::SolveType>(
              {Moose::ST_NEWTON, Moose::ST_JFNK, Moose::ST_PJFNK, Moose::ST_FD, Moose::ST_LINEAR}),
          ","),
      "NEWTON,JFNK,Preconditioned JFNK,FD,Linear");
}

TEST(Conversion, ExecFlagType)
{
  std::vector<ExecFlagType> flags = {EXEC_INITIAL,
                                     EXEC_LINEAR,
                                     EXEC_NONLINEAR,
                                     EXEC_TIMESTEP_END,
                                     EXEC_TIMESTEP_BEGIN,
                                     EXEC_MULTIAPP_FIXED_POINT_BEGIN,
                                     EXEC_MULTIAPP_FIXED_POINT_END,
                                     EXEC_CUSTOM,
                                     EXEC_FINAL,
                                     EXEC_FORCED,
                                     EXEC_FAILED,
                                     EXEC_SUBDOMAIN,
                                     EXEC_NONE};

  EXPECT_EQ(Moose::stringify(flags, ", "),
            "INITIAL, LINEAR, NONLINEAR, TIMESTEP_END, TIMESTEP_BEGIN, MULTIAPP_FIXED_POINT_BEGIN, "
            "MULTIAPP_FIXED_POINT_END, CUSTOM, FINAL, FORCED, FAILED, SUBDOMAIN, NONE");

  EXPECT_EQ(Moose::stringify(EXEC_INITIAL), "INITIAL");
  EXPECT_EQ(Moose::stringify(EXEC_LINEAR), "LINEAR");
  EXPECT_EQ(Moose::stringify(EXEC_NONLINEAR), "NONLINEAR");
  EXPECT_EQ(Moose::stringify(EXEC_TIMESTEP_END), "TIMESTEP_END");
  EXPECT_EQ(Moose::stringify(EXEC_TIMESTEP_BEGIN), "TIMESTEP_BEGIN");
  EXPECT_EQ(Moose::stringify(EXEC_MULTIAPP_FIXED_POINT_BEGIN), "MULTIAPP_FIXED_POINT_BEGIN");
  EXPECT_EQ(Moose::stringify(EXEC_MULTIAPP_FIXED_POINT_END), "MULTIAPP_FIXED_POINT_END");
  EXPECT_EQ(Moose::stringify(EXEC_CUSTOM), "CUSTOM");
  EXPECT_EQ(Moose::stringify(EXEC_FINAL), "FINAL");
  EXPECT_EQ(Moose::stringify(EXEC_FINAL), "FINAL");
  EXPECT_EQ(Moose::stringify(EXEC_FORCED), "FORCED");
  EXPECT_EQ(Moose::stringify(EXEC_SUBDOMAIN), "SUBDOMAIN");
  EXPECT_EQ(Moose::stringify(EXEC_NONE), "NONE");
}

TEST(Conversion, stringifyExact)
{
  EXPECT_EQ(Moose::stringifyExact(libMesh::pi), "3.1415926535897931");

  // test roundtrips
  for (auto v : std::vector<Real>({libMesh::pi,
                                   1.0 / 17.0,
                                   std::exp(1.0),
                                   std::sqrt(2.0),
                                   std::cbrt(3.0),
                                   std::log(10.0)}))
    EXPECT_EQ(std::stod(Moose::stringifyExact(v)), v);
}
