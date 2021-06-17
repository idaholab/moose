//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSQCriterionAux.h"

registerMooseObject("NavierStokesApp", INSQCriterionAux);

InputParameters
INSQCriterionAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("This class computes the Q criterion, a scalar which"
                             "aids in vortex identification in turbulent flows");
  params.addCoupledVar("velocity", "The velocity component");
  return params;
}

INSQCriterionAux::INSQCriterionAux(const InputParameters & parameters)
  : AuxKernel(parameters), _grad_velocity(coupledVectorGradient("velocity"))
{
}

Real
INSQCriterionAux::computeValue()
{
  const Real symm_part = 2.0 * Utility::pow<2>(_grad_velocity[_qp](0, 0)) +
                         2.0 * Utility::pow<2>(_grad_velocity[_qp](1, 1)) +
                         2.0 * Utility::pow<2>(_grad_velocity[_qp](2, 2)) +
                         Utility::pow<2>(_grad_velocity[_qp](0, 2) + _grad_velocity[_qp](2, 0)) +
                         Utility::pow<2>(_grad_velocity[_qp](0, 1) + _grad_velocity[_qp](1, 0)) +
                         Utility::pow<2>(_grad_velocity[_qp](1, 2) + _grad_velocity[_qp](2, 1));
  const Real antisymm_part =
      Utility::pow<2>(_grad_velocity[_qp](0, 2) - _grad_velocity[_qp](2, 0)) +
      Utility::pow<2>(_grad_velocity[_qp](0, 1) - _grad_velocity[_qp](1, 0)) +
      Utility::pow<2>(_grad_velocity[_qp](1, 2) - _grad_velocity[_qp](2, 1));
  return 0.5 * (antisymm_part - symm_part);
}
