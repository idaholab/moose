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
#include "neml2/tensors/functions/det.h"
#include "neml2/tensors/functions/pow.h"
#include "neml2/tensors/functions/sum.h"

// MOOSE includes
#include "NEML2DeformationGradient.h"

registerMooseObject("SolidMechanicsApp", NEML2DeformationGradient);

InputParameters
NEML2DeformationGradient::validParams()
{
  InputParameters params = NEML2SmallStrain::validParams();
  params.addClassDescription(
      "This user object calculates the deformation gradient $F = I + \\nabla_0 u$ from "
      "displacement gradients on the reference configuration, for use with total-Lagrangian "
      "NEML2 models.");
  params.addParam<bool>(
      "stabilize_strain",
      false,
      "Apply the F-bar volumetric correction: rescale each quadrature point's F to the "
      "volume-weighted element-average volume change. Required for near-incompressible flow "
      "(e.g. plasticity) on linear quads/hexes, which otherwise lock volumetrically. Matches "
      "ComputeLagrangianStrainBase with stabilize_strain = true.");
  return params;
}

NEML2DeformationGradient::NEML2DeformationGradient(const InputParameters & parameters)
  : NEML2SmallStrain(parameters), _stabilize_strain(getParam<bool>("stabilize_strain"))
{
}

void
NEML2DeformationGradient::computeF()
{
  // gradient of displacements
  const auto & dux = *_grad_disp_x;
  auto duy = _grad_disp_y ? *_grad_disp_y : neml2::Tensor::zeros_like(dux);
  auto duz = _grad_disp_z ? *_grad_disp_z : neml2::Tensor::zeros_like(dux);
  auto du = neml2::R2(neml2::base_stack({dux, duy, duz}, -2));

  // F = I + grad_0 u
  _output = du + neml2::R2::identity(du.options());
}

void
NEML2DeformationGradient::applyFBar()
{
  // volume-weighted element average of F: F_avg = sum_qp(w F) / sum_qp(w), with the
  // reference-configuration weights (including the coordinate transformation, e.g. 2*pi*r
  // in RZ) cached by the assembly
  auto w = _neml2_assembly.JxWxT().base_unsqueeze(0).base_unsqueeze(0); // (nelem, nqp; 1, 1)
  auto F = neml2::Tensor(_output);                                      // (nelem, nqp; 3, 3)
  auto F_avg =
      neml2::dynamic_sum(F * w, -1) / neml2::dynamic_sum(w, -1); // (nelem; 3, 3) / (nelem; 1, 1)

  // F <- F (det F_avg / det F)^(1/3)
  auto ratio = neml2::det(neml2::R2(F_avg)).dynamic_unsqueeze(-1) / neml2::det(neml2::R2(F));
  auto scale = neml2::pow(ratio, 1.0 / 3.0).base_unsqueeze(0).base_unsqueeze(0);
  _output = neml2::R2(F * scale);
}

void
NEML2DeformationGradient::forward()
{
  computeF();
  if (_stabilize_strain)
    applyFBar();
}

#endif
