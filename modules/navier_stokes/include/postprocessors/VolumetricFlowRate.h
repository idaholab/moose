/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
