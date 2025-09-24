//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AdvectionIPHDGOutflowBC.h

/**
 * Implements an outflow boundary condition for use with a hybridized discretization of
 * the Advection equation
 */
class MassAdvectionIPHDGOutflowBC : public AdvectionIPHDGOutflowBC
{
public:
  static InputParameters validParams();
  MassAdvectionIPHDGOutflowBC(const InputParameters & parameters);

protected:
  /**
   * compute the AD residuals
   */
  virtual void compute() override;
};
