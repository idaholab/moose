//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

// NEML2 includes
#include "neml2/tensors/functions/stack.h"

// MOOSE includes
#include "NEML2SmallStrain.h"

registerMooseObject("SolidMechanicsApp", NEML2SmallStrain);

InputParameters
NEML2SmallStrain::validParams()
{
  InputParameters params = NEML2PreKernel::validParams();
  params.addClassDescription(
      "This user object calculates the small strain from displacement gradients. "
      "It requires 1 to 3 displacement variables, which are used to compute the strain tensor.");
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "displacements", "The displacements to use to calculate the strain.");
  return params;
}

NEML2SmallStrain::NEML2SmallStrain(const InputParameters & parameters) : NEML2PreKernel(parameters)
{
  auto disp_vars = getParam<std::vector<NonlinearVariableName>>("displacements");
  if (disp_vars.size() < 1 || disp_vars.size() > 3)
    mooseError("NEML2SmallStrain requires 1 to 3 displacement variables, got ", disp_vars.size());

  _grad_disp_x = &_fe.getGradient(disp_vars[0]);
  _grad_disp_y = disp_vars.size() >= 2 ? &_fe.getGradient(disp_vars[1]) : nullptr;
  _grad_disp_z = disp_vars.size() >= 3 ? &_fe.getGradient(disp_vars[2]) : nullptr;
}

void
NEML2SmallStrain::forward()
{
  // gradient of displacements
  const auto & dux = *_grad_disp_x;
  auto duy = _grad_disp_y ? *_grad_disp_y : neml2::Tensor::zeros_like(dux);
  auto duz = _grad_disp_z ? *_grad_disp_z : neml2::Tensor::zeros_like(dux);
  auto du = neml2::R2(neml2::base_stack({dux, duy, duz}, -2));

  // strain = 0.5 * (grad_u + grad_u^T), neml2::SR2 handles the symmetrization
  _output = neml2::SR2(du);
}

#endif
