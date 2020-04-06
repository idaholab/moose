//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

/**
 * Boundary condition of a flux type \f$ <\vec q * \vec n, v> \f$.
 *
 * User needs to provide vector \f$ \vec q \f$.
 */
class FluxBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  FluxBC(const InputParameters & params);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  virtual RealGradient computeQpFluxResidual() = 0;
  virtual RealGradient computeQpFluxJacobian() = 0;
};
