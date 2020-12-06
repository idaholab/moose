//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <array>

#include "SmoothCircleBaseIC.h"
#include "DelimitedFileReader.h"

/**
 * Reads multiple circles from a text file with the columns labeled
 * x   y   z   r. It expects the file to have a one-line header.
 * Applies all of the circles to the same variable.
 */
class SmoothCircleFromFileIC : public SmoothCircleBaseIC
{
public:
  static InputParameters validParams();

  SmoothCircleFromFileIC(const InputParameters & parameters);

protected:
  virtual void computeCircleRadii();
  virtual void computeCircleCenters();

  enum COLS
  {
    X,
    Y,
    Z,
    R
  };

  std::array<int, 4> _col_map = {{-1, -1, -1, -1}};
  std::vector<std::vector<Real>> _data;
  FileName _file_name;
  MooseUtils::DelimitedFileReader _txt_reader;
  std::vector<std::string> _col_names;
  unsigned int _n_circles;
};
