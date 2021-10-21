//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"

class PiecewiseBilinear;
template <typename>
class ColumnMajorMatrixTempl;
typedef ColumnMajorMatrixTempl<Real> ColumnMajorMatrix;
class BilinearInterpolation;

template <>
InputParameters validParams<PiecewiseBilinear>();

//c
//c PiecewiseBilinear reads a file containing a (x, y, z) data table
//c where (x,y) is a descretized or measured point and z is the measured value at (x,y) point.
//c  Then, function PiecewiseBilinear will evalaute any (x',y') points by making
//c  a bilinear interpolation based on these measured/input (x,y) table.
//c  For example, you measure temperatures on a rectangular meshed by 2x2 elements, which has total 9
//c  points/nodes to be measured. Assuming that the (x, y) coordinates of these 9 points/nodes are
//c  (1000.0,7000.0),(2000.0,7000.0),(3000.0,7000.0),(1000.0,8500.0),(2000,8500.0),(3000.0,8500.0),
//c  (1000.0,9600.0),(2000.0,9600.0),(3000.0,9600.0) and the corresponding measured temparatures are:
//c  20.000,20.300,20.800,30.030,32.300,34.800,40.100,42.500,45.030.
//c The input data in the file (temparature.csv) should be below:

// Temparature,1000.0,2000.0,3000.0
//      7000.0,20.000,20.300,20.800
//      8500.0,30.030,32.300,34.800
//      9600.0,40.100,42.500,45.030

//in the function block of the inpute file: data_file = temperature.csv

class PiecewiseBilinear : public Function
{
public:
  static InputParameters validParams();

  PiecewiseBilinear(const InputParameters & parameters);

  // Necessary for using forward declaration of BilinearInterpolation in std::unique_ptr
  virtual ~PiecewiseBilinear();

  /**
   * This function will return a value based on the first input argument only.
   */
  virtual Real value(Real t, const Point & pt) const override;

private:
  std::unique_ptr<BilinearInterpolation> _bilinear_interp;
  const std::string _data_file_name;
  const int _axis;
  const int _yaxis;
  const int _xaxis;
  const bool _axisValid;
  const bool _yaxisValid;
  const bool _xaxisValid;
  const Real _scale_factor;
  const bool _radial;

  void parse(std::vector<Real> & x, std::vector<Real> & y, ColumnMajorMatrix & z);
};
