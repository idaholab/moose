//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CSVINTERPOLATOR_H
#define CSVINTERPOLATOR_H

#include "GeneralUserObject.h"
#include "BilinearInterpolation.h"

class CSVInterpolator;

template <>
InputParameters validParams<CSVInterpolator>();

/**
 * This class reads csv files and creates a bilinear interpolation object to interpolate that data
 * as a function of position (in user provided direction) and time. This bilinear interpolation
 * object can then be called by a meterial to set values to a material property at each qp at any
 * time.
 */

class CSVInterpolator : public GeneralUserObject
{
public:
  CSVInterpolator(const InputParameters & parameters);

  /// Obtain the number of variables extracted from the csv files
  unsigned int getNumberOfVariables() const { return _variable_vectors.size(); };

  /// Obtain the value of the variables for the given x (position) and y (time) from the bilinear interpolation object
  std::vector<Real> getValue(const Real & x, const Real & y) const;

protected:
  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override{};

  /// Obtain all file names following the same pattern
  std::vector<std::string> glob(const std::string & pattern);

  /// Pattern for the csv files
  const std::string _pattern;

  /// Position along this component direction is extracted from the csv files
  const unsigned int _from_component;

  /// List of variable names to be extracted from the csv files
  const std::vector<std::string> _variable_vectors;

  /// Vector of pointers to bilinear interpolation objects. Each object corresponds to one variable
  std::vector<std::unique_ptr<BilinearInterpolation>> _bilinear_interp;
};

#endif // CSVINTERPOLATOR_H
