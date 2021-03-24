//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumPressure.h"
#include "PINSFVSuperficialVelocityVariable.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumPressure);

InputParameters
PINSFVMomentumPressure::validParams()
{
  InputParameters params = INSFVMomentumPressure::validParams();
  params.addClassDescription("Introduces the coupled pressure term $eps \nabla P$ into the "
                             "Navier-Stokes porous media momentum equation.");
  params.addRequiredCoupledVar("porosity", "Porosity auxiliary variable");
  params.addParam<bool>("smooth_porosity", false, "Whether the porosity has no discontinuities");
  params.addParam<Real>("max_eps_gradient", 1e-12, "Maximum porosity gradient before considering a discontinuity exists");

  return params;
}

PINSFVMomentumPressure::PINSFVMomentumPressure(const InputParameters & params)
  : INSFVMomentumPressure(params),
    _eps(coupledValue("porosity")),
    _eps_var(dynamic_cast<const MooseVariableFV<Real> *>(getFieldVar("porosity", 0))),
    _smooth_porosity(getParam<bool>("smooth_porosity")),
    _max_eps_gradient(getParam<Real>("max_eps_gradient"))
{
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError("PINSFVMomentumPressure may only be used with a superficial velocity "
               "variable, of variable type PINSFVSuperficialVelocityVariable.");
}

ADReal
PINSFVMomentumPressure::computeQpResidual()
{
  if (_smooth_porosity || MetaPhysicL::raw_value(_eps_var->adGradSln(_current_elem)).norm() < _max_eps_gradient)
    return _eps[_qp] * INSFVMomentumPressure::computeQpResidual();
  else
    return INSFVMomentumPressure::computeQpResidual();
}
