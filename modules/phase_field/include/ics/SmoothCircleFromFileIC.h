/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SMOOTHCIRCLEFROMFILE_H
#define SMOOTHCIRCLEFROMFILE_H

#include <array>

#include "SmoothCircleBaseIC.h"
#include "DelimitedFileReader.h"

// Forward Declarations
class SmoothCircleFromFileIC;

template <>
InputParameters validParams<SmoothCircleFromFileIC>();

/**
 * Reads multiple circles from a text file with the columns labeled
 * x   y   z   r. It expects the file to have a one-line header.
 * Applies all of the circles to the same variable.
**/

class SmoothCircleFromFileIC : public SmoothCircleBaseIC
{
public:
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
  // Double braces in array initializer are workaround for bug in gcc 4.9.2
  std::array<int, 4> _col_map = {{-1, -1, -1, -1}};
  std::vector<std::vector<Real>> _data;
  FileName _file_name;
  MooseUtils::DelimitedFileReader _txt_reader;
  std::vector<std::string> _col_names;
  unsigned int _n_circles;
};

#endif // SMOOTHCIRCLEFROMFILE_H
