//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MDFluidIntegratedBCBase.h"
#include "Function.h"

/* A specific BC for the mass (pressure) equation  */
class MDFluidMassBC : public MDFluidIntegratedBCBase
{
public:
  static InputParameters validParams();

  MDFluidMassBC(const InputParameters & parameters);
  virtual ~MDFluidMassBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  bool _has_vfn;
  bool _has_vpps;
  const Function * _velocity_fn;
  std::string _v_pps_name;
};
