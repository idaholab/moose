//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMixingLengthTurbulentViscosityAux.h"
#include "INSFVVelocityVariable.h"

registerMooseObject("NavierStokesApp", INSFVMixingLengthTurbulentViscosityAux);

InputParameters
INSFVMixingLengthTurbulentViscosityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes the turbulent viscosity for the mixing length model.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredCoupledVar("mixing_length", "Turbulent eddy mixing length.");
  return params;
}

INSFVMixingLengthTurbulentViscosityAux::INSFVMixingLengthTurbulentViscosityAux(
    const InputParameters & params)
  : AuxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _mixing_len(coupledValue("mixing_length"))
{
  if (!_u_var)
    paramError("u", "the u velocity must be an INSFVVelocityVariable.");

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  if (_dim >= 3 && !_w_var)
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");
}

Real
INSFVMixingLengthTurbulentViscosityAux::computeValue()
{
  constexpr Real offset = 1e-15; // prevents explosion of sqrt(x) derivative to infinity
  const Elem & elem = *_current_elem;
  const auto state = determineState();

  const auto & grad_u = _u_var->adGradSln(&elem, state);
  ADReal symmetric_strain_tensor_norm = 2.0 * Utility::pow<2>(grad_u(0));
  if (_dim >= 2)
  {
    const auto & grad_v = _v_var->adGradSln(&elem, state);
    symmetric_strain_tensor_norm +=
        2.0 * Utility::pow<2>(grad_v(1)) + Utility::pow<2>(grad_v(0) + grad_u(1));
    if (_dim >= 3)
    {
      const auto & grad_w = _w_var->adGradSln(&elem, state);
      symmetric_strain_tensor_norm += 2.0 * Utility::pow<2>(grad_w(2)) +
                                      Utility::pow<2>(grad_u(2) + grad_w(0)) +
                                      Utility::pow<2>(grad_v(2) + grad_w(1));
    }
  }

  symmetric_strain_tensor_norm = std::sqrt(symmetric_strain_tensor_norm + offset);

  // Compute the eddy diffusivitiy
  ADReal eddy_diff = symmetric_strain_tensor_norm * _mixing_len[_qp] * _mixing_len[_qp];

  // Return the turbulent stress contribution to the momentum equation
  return eddy_diff.value();
}
