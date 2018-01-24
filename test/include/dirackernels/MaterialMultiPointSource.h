//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALMULTIPOINTSOURCE_H
#define MATERIALMULTIPOINTSOURCE_H

// Moose Includes
#include "DiracKernel.h"

// Forward Declarations
class MaterialMultiPointSource;

template <>
InputParameters validParams<MaterialMultiPointSource>();

/**
 * Similar to the ConstantPointSource, but evaluates a Material
 * property at the point source location instead of using a constant
 * value.
 */
class MaterialMultiPointSource : public DiracKernel
{
public:
  MaterialMultiPointSource(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  std::vector<Point> _points;

  const MaterialProperty<Real> & _value;
};

#endif // MATERIALMULTIPOINTSOURCE_H
