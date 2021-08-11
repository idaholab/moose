//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NewmarkBetaContactTimeKernel.h"
#include "SubProblem.h"
#include "TimeIntegrator.h"
#include "NonlinearSystemBase.h"

registerMooseObject("ContactApp", NewmarkBetaContactTimeKernel);

InputParameters
NewmarkBetaContactTimeKernel::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
  params.addClassDescription("Calculates the residual for the inertial force.");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<MaterialPropertyName>(
      "density", "density", "Name of Material Property that provides the density");
  return params;
}

NewmarkBetaContactTimeKernel::NewmarkBetaContactTimeKernel(const InputParameters & parameters)
  : ADKernel(parameters), _density(getMaterialProperty<Real>("density")), _u_old(_var.slnOld())
{
}

ADReal
NewmarkBetaContactTimeKernel::computeQpResidual()
{
  return _density[_qp] * 2. * (_u[_qp] - _u_old[_qp]) / (_dt * _dt);
}
