/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MooseUtils.h"
#include "SplitFileNameTest.h"
#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION(SplitFileNameTest);

void
SplitFileNameTest::invalidName()
{
  std::string full = "/this/is/not/valid/";
  std::pair<std::string, std::string> split = MooseUtils::splitFileName(full);
}

void
SplitFileNameTest::validName()
{
  std::string full = "/this/is/valid.txt";
  std::pair<std::string, std::string> split = MooseUtils::splitFileName(full);

  CPPUNIT_ASSERT(split.first.compare("/this/is") == 0);
  CPPUNIT_ASSERT(split.second.compare("valid.txt") == 0);

  full = "valid.txt";
  split = MooseUtils::splitFileName(full);
  CPPUNIT_ASSERT(split.first.compare(".") == 0);
  CPPUNIT_ASSERT(split.second.compare("valid.txt") == 0);
}
