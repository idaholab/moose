//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVAdvection.h"
#include "Steady.h"

registerADMooseObject("MooseApp", FVAdvection);

InputParameters
FVAdvection::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription(
      "Residual contribution from advection operator for finite volume method.");
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  params += Moose::FV::advectedInterpolationParameter();
  return params;
}

FVAdvection::FVAdvection(const InputParameters & params)
  : FVFluxKernel(params), _velocity(getParam<RealVectorValue>("velocity"))
{
  const bool need_more_ghosting =
      Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");
  if (need_more_ghosting && _tid == 0)
  {
    adjustRMGhostLayers(std::max((unsigned short)(2), _pars.get<unsigned short>("ghost_layers")));

    // If we need more ghosting, then we are a second-order nonlinear limiting scheme whose stencil
    // is liable to change upon wind-direction change. Consequently we need to tell our problem that
    // it's ok to have new nonzeros which may crop-up after PETSc has shrunk the matrix memory
    getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")
        ->setErrorOnJacobianNonzeroReallocation(false);
  }

  if (dynamic_cast<Steady *>(_app.getExecutioner()))
  {
    const MooseEnum not_available_with_steady("sou min_mod vanLeer quick venkatakrishnan");
    const std::string chosen_scheme =
        static_cast<std::string>(getParam<MooseEnum>("advected_interp_method"));
    if (not_available_with_steady.find(chosen_scheme) != not_available_with_steady.items().end())
      paramError("advected_interp_method",
                 "The given advected interpolation cannot be used with steady-state runs!");
  }
}

ADReal
FVAdvection::computeQpResidual()
{
  const auto state = determineState();
  const auto & limiter_time = _subproblem.isTransient()
                                  ? Moose::StateArg(1, Moose::SolutionIterationType::Time)
                                  : Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear);

  const bool elem_is_upwind = _velocity * _normal >= 0;
  const auto face = makeFace(*_face_info,
                             Moose::FV::limiterType(_advected_interp_method),
                             elem_is_upwind,
                             false,
                             &limiter_time);
  ADReal u_interface = _var(face, state);

  return _normal * _velocity * u_interface;
}
