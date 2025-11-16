//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAdvectionDiffusionFunctorRobinBC.h"

registerMooseObject("MooseApp", LinearFVAdvectionDiffusionFunctorRobinBC);

InputParameters
LinearFVAdvectionDiffusionFunctorRobinBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionFunctorRobinBCBase::validParams();
  params.addClassDescription(
      "Adds a Robin BC of the form \\alpha * \\nabla \\phi*n + \\beta * \\phi = \\gamma, "
      "which can be used for the assembly of linear "
      "finite volume system and whose face values are determined using "
      "three functors. This kernel is "
      "only designed to work with advection-diffusion problems.");
  params.addParam<MooseFunctorName>(
      "alpha", 1.0, "Functor for the coefficient of the normal gradient term.");
  params.addParam<MooseFunctorName>("beta", 1.0, "Functor for the coefficient of the scalar term.");
  params.addParam<MooseFunctorName>(
      "gamma", 1.0, "Functor for the constant term on the RHS of the Robin BC.");
  return params;
}

LinearFVAdvectionDiffusionFunctorRobinBC::LinearFVAdvectionDiffusionFunctorRobinBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionFunctorRobinBCBase(parameters),
    _alpha(getFunctor<Real>("alpha")),
    _beta(getFunctor<Real>("beta")),
    _gamma(getFunctor<Real>("gamma"))
{
  _var.computeCellGradients();

  if (_alpha.isConstant())
  {
    // We check if we can parse the value to a number and if yes, we throw an error if it is 0
    std::istringstream ss(getParam<MooseFunctorName>("alpha"));
    Real real_value;
    if (ss >> real_value && ss.eof())
      if (MooseUtils::isZero(real_value))
        paramError("alpha",
                   "This value shall not be 0. Use a Dirichlet boundary condition instead!");
  }
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::getAlpha(Moose::FaceArg face, Moose::StateArg state) const
{
  return _alpha(face, state);
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::getBeta(Moose::FaceArg face, Moose::StateArg state) const
{
  return _beta(face, state);
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::getGamma(Moose::FaceArg face, Moose::StateArg state) const
{
  return _gamma(face, state);
}
