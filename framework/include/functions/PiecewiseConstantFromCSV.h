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
#include "PropertyReadFile.h"

/**
 * Function which provides a piecewise constant, in space, field from a CSV file
 */
class PiecewiseConstantFromCSV : public Function
{
public:
  static InputParameters validParams();

  PiecewiseConstantFromCSV(const InputParameters & parameters);

  /**
   * Get the value of the function based on the data in the CSV file
   * \param t The time (unused)
   * \param pt The point in space (x,y,z)
   * \return The value of the function
   */
  virtual Real value(Real t, const Point & pt) const override;
  using Function::value;

  virtual Real timeDerivative(Real, const Point &) const override { return 0; };

protected:
  void initialSetup() override;

  /// A user object that takes care of reading the CSV file
  const PropertyReadFile * _read_prop_user_object;

  /// The column number of interest in the CSV file
  const unsigned int _column_number;

  /// Type of read - element, grain, or block
  const PropertyReadFile::ReadTypeEnum _read_type;

  /// The point locator is used when values are sorted by elements or blocks in the CSV
  std::unique_ptr<PointLocatorBase> _point_locator;
};
