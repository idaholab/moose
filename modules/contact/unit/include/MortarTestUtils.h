//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>

// Compute the absolute path to an input file stored alongside the calling source file in
// ../inputs/ by navigating from __FILE__ (the absolute path to that .C file).
inline std::string
inputPath(const std::string & file, const std::string & name)
{
  const auto pos = file.rfind("/src/");
  return file.substr(0, pos + 1) + "inputs/" + name;
}
