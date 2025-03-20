//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShearStressNormAux.h"
#include "NavierStokesMethods.h"
#include "NonlinearSystemBase.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", ShearStressNormAux);

InputParameters
ShearStressNormAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Calculates the turbulent viscosity according to the k-epsilon model.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  return params;
}

ShearStressNormAux::ShearStressNormAux(const InputParameters & params)
  : AuxKernel(params),
    _u_var(getFunctor<Real>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<Real>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<Real>("w")) : nullptr)
{
}

Real
ShearStressNormAux::computeValue()
{
  return NS::computeShearStrainRateNormSquared<Real>(
      _u_var, _v_var, _w_var, makeElemArg(_current_elem), determineState());
}
