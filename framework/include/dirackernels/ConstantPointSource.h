//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "DiracKernel.h"

// Forward Declarations
class ConstantPointSource;

template <>
InputParameters validParams<ConstantPointSource>();

/**
 * A ConstantPointSource DiracKernel is used to create constant point sources.
 * Coordinates and values for the point source can be given in the input or read from file
 */
class ConstantPointSource : public DiracKernel
{
public:
  static InputParameters validParams();

  /// map to associate points with their value in computeQpResidual
  ConstantPointSource(const InputParameters & parameters);

  virtual void addPoints() override;

protected:
  virtual Real computeQpResidual() override;
  /// map to associate points with their value in computeQpResidual
  std::map<Point, Real> _point_to_value;

private:
  /// helper to read from file
  void readFromFile(std::vector<Real> & value, std::vector<Real> & point_param);
};
