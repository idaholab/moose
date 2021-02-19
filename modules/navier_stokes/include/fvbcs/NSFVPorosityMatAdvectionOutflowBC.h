//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVMatAdvectionOutflowBC.h"

class NSFVPorosityMatAdvectionOutflowBC : public FVMatAdvectionOutflowBC
{
public:
  static InputParameters validParams();
  NSFVPorosityMatAdvectionOutflowBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// The porosity on the element
  const MaterialProperty<Real> & _eps_elem;

  /// The porosity on the neighbor
  const MaterialProperty<Real> & _eps_neighbor;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _one_over_porosity_interp_method;
};
