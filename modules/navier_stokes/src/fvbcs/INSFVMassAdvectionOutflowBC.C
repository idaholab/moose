//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMassAdvectionOutflowBC.h"
#include "INSFVVelocityVariable.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "NS.h"

registerADMooseObject("NavierStokesApp", INSFVMassAdvectionOutflowBC);

InputParameters
INSFVMassAdvectionOutflowBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params += INSFVFullyDevelopedFlowBC::validParams();
  params.addClassDescription("Outflow boundary condition for advecting mass.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The functor for the density");
  params.declareControllable("rho");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  return params;
}

INSFVMassAdvectionOutflowBC::INSFVMassAdvectionOutflowBC(const InputParameters & params)
  : FVFluxBC(params),
    INSFVFullyDevelopedFlowBC(params),
    _rho(getFunctor<ADReal>(NS::density)),
    _u(getFunctor<ADReal>("u")),
    _v(isParamValid("v") ? &getFunctor<ADReal>("v") : nullptr),
    _w(isParamValid("w") ? &getFunctor<ADReal>("w") : nullptr),
    _dim(_subproblem.mesh().dimension())
{
  if (_dim >= 2 && !_v)
    mooseError(
        "In two or more dimensions, the v velocity must be supplied using the 'v' parameter");
  if (_dim >= 3 && !_w)
    mooseError("In threedimensions, the w velocity must be supplied using the 'w' parameter");
}

ADReal
INSFVMassAdvectionOutflowBC::computeQpResidual()
{
  const auto boundary_face = singleSidedFaceArg();
  const auto state = determineState();

  ADRealVectorValue v(_u(boundary_face, state));
  if (_v)
    v(1) = (*_v)(boundary_face, state);
  if (_w)
    v(2) = (*_w)(boundary_face, state);

  return _normal * v * _rho(boundary_face, state);
}
