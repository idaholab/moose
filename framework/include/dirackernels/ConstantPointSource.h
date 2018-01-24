//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONSTANTPOINTSOURCE_H
#define CONSTANTPOINTSOURCE_H

// Moose Includes
#include "DiracKernel.h"

// Forward Declarations
class ConstantPointSource;

template <>
InputParameters validParams<ConstantPointSource>();

/**
 * TOOD
 */
class ConstantPointSource : public DiracKernel
{
public:
  ConstantPointSource(const InputParameters & parameters);

  virtual void addPoints() override;

protected:
  virtual Real computeQpResidual() override;

  const Real & _value;
  std::vector<Real> _point_param;
  Point _p;
};

#endif // CONSTANTPOINTSOURCE_H
