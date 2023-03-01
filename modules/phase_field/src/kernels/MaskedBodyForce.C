//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaskedBodyForce.h"
#include "Function.h"

registerMooseObject("PhaseFieldApp", MaskedBodyForce);

InputParameters
MaskedBodyForce::validParams()
{
  InputParameters params = BodyForce::validParams();
  params.addClassDescription("Kernel that defines a body force modified by a material mask");
  params.addParam<MaterialPropertyName>("mask", "Material property defining the mask");
  params.addCoupledVar("args", "Vector of nonlinear variable arguments this object depends on");
  params.deprecateCoupledVar("args", "coupled_variables", "02/27/2024");
  return params;
}

MaskedBodyForce::MaskedBodyForce(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<BodyForce>>(parameters),
    _mask(getMaterialProperty<Real>("mask")),
    _v_name(_var.name()),
    _dmaskdv(getMaterialPropertyDerivative<Real>("mask", _v_name)),
    _dmaskdarg(_n_args)
{
  // Get derivatives of mask wrt coupled variables
  for (unsigned int i = 0; i < _n_args; ++i)
    _dmaskdarg[i] = &getMaterialPropertyDerivative<Real>("mask", i);
}

void
MaskedBodyForce::initialSetup()
{
  validateNonlinearCoupling<Real>("mask");
}

Real
MaskedBodyForce::computeQpResidual()
{
  return BodyForce::computeQpResidual() * _mask[_qp];
}

Real
MaskedBodyForce::computeQpJacobian()
{
  return _dmaskdv[_qp] * BodyForce::computeQpResidual() * _phi[_j][_qp];
}

Real
MaskedBodyForce::computeQpOffDiagJacobian(unsigned int jvar)
{
  const unsigned int cvar = mapJvarToCvar(jvar);
  return (*_dmaskdarg[cvar])[_qp] * BodyForce::computeQpResidual() * _phi[_j][_qp];
}
