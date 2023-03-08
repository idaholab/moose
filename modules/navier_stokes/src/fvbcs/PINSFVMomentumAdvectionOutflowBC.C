//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumAdvectionOutflowBC.h"
#include "PINSFVSuperficialVelocityVariable.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumAdvectionOutflowBC);

InputParameters
PINSFVMomentumAdvectionOutflowBC::validParams()
{
  InputParameters params = INSFVMomentumAdvectionOutflowBC::validParams();
  params.addRequiredParam<MooseFunctorName>("u", "The superficial velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The superficial velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The superficial velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "The porosity");
  params.addClassDescription(
      "Outflow boundary condition for advecting momentum in the porous media momentum equation. "
      "This will impose a zero normal gradient on the boundary velocity.");
  return params;
}

PINSFVMomentumAdvectionOutflowBC::PINSFVMomentumAdvectionOutflowBC(const InputParameters & params)
  : INSFVMomentumAdvectionOutflowBC(params), _eps(getFunctor<ADReal>("porosity"))
{
}
