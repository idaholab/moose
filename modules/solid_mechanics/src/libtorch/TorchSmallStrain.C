//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

// MOOSE includes
#include "TorchSmallStrain.h"
#include "TorchFEMUtils.h"

registerMooseObject("SolidMechanicsApp", TorchSmallStrain);

InputParameters
TorchSmallStrain::validParams()
{
  InputParameters params = TorchPreKernel::validParams();
  params.addClassDescription(
      "This user object calculates the small strain from displacement gradients. "
      "It requires 1 to 3 displacement variables, which are used to compute the strain tensor.");
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "displacements", "The displacements to use to calculate the strain.");
  return params;
}

TorchSmallStrain::TorchSmallStrain(const InputParameters & parameters) : TorchPreKernel(parameters)
{
  auto disp_vars = getParam<std::vector<NonlinearVariableName>>("displacements");
  if (disp_vars.size() < 1 || disp_vars.size() > 3)
    mooseError("TorchSmallStrain requires 1 to 3 displacement variables, got ", disp_vars.size());

  _grad_disp_x = &_fe.getGradient(disp_vars[0]);
  _grad_disp_y = disp_vars.size() >= 2 ? &_fe.getGradient(disp_vars[1]) : nullptr;
  _grad_disp_z = disp_vars.size() >= 3 ? &_fe.getGradient(disp_vars[2]) : nullptr;
}

void
TorchSmallStrain::forward()
{
  // gradient of displacements, each (nelem, nqp, 3)
  const auto & dux = *_grad_disp_x;
  auto duy = _grad_disp_y ? *_grad_disp_y : at::zeros_like(dux);
  auto duz = _grad_disp_z ? *_grad_disp_z : at::zeros_like(dux);

  // displacement gradient du[..., i, j] = d u_i / d x_j, shape (nelem, nqp, 3, 3)
  auto du = at::stack({dux, duy, duz}, -2);

  // strain = 0.5 * (grad_u + grad_u^T), stored in 6-component Mandel form
  _output = TorchFEM::fullToMandel(0.5 * (du + du.transpose(-2, -1)));
}

#endif // NEML2_ENABLED
