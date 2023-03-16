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

/**
 * A VectorConstantPointSource DiracKernel is used to add a constant source term
 * at a point.
 */
class VectorConstantPointSource : public VectorDiracKernel
{
public:
  static InputParameters validParams();

  VectorConstantPointSource(const InputParameters & parameters);

  virtual void addPoints() override;

protected:
  virtual Real computeQpResidual() override;

  const RealVectorValue & _values;
  std::vector<Real> _point_param;
  Point _p;
};
