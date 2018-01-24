//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VOLUMETRICFLOWRATE_H
#define VOLUMETRICFLOWRATE_H

// MOOSE includes
#include "SideIntegralPostprocessor.h"

// Forward Declarations
class VolumetricFlowRate;

template <>
InputParameters validParams<VolumetricFlowRate>();

/**
 * This postprocessor computes the volumetric flow rate through a boundary.
 */
class VolumetricFlowRate : public SideIntegralPostprocessor
{
public:
  VolumetricFlowRate(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  const VariableValue & _vel_x;
  const VariableValue & _vel_y;
  const VariableValue & _vel_z;
};

#endif // VOLUMETRICFLOWRATE_H
