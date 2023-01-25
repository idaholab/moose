//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFEFluidIntegratedBCBase.h"
#include "Function.h"

/**
 * A specific BC for the mass (pressure) equation
 */
class INSFEFluidMassBC : public INSFEFluidIntegratedBCBase
{
public:
  static InputParameters validParams();

  INSFEFluidMassBC(const InputParameters & parameters);
  virtual ~INSFEFluidMassBC() {}

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  bool _has_vfn;
  bool _has_vpps;
  const Function * _velocity_fn;
  std::string _v_pps_name;
};
