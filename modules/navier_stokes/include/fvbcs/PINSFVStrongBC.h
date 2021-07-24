//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"
#include "INSFVFlowBC.h"

class PINSFVStrongBC : public FVFluxBC, public INSFVFlowBC
{
public:
  static InputParameters validParams();
  PINSFVStrongBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const FunctorInterface<ADReal> & _sup_vel_x;
  const FunctorInterface<ADReal> * const _sup_vel_y;
  const FunctorInterface<ADReal> * const _sup_vel_z;
  const FunctorInterface<ADReal> & _pressure;
  const FunctorMaterialProperty<ADReal> & _rho;
  const FunctorInterface<ADReal> & _eps;
  const MooseEnum _eqn;
  const unsigned int _index;
};
