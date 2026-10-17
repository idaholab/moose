//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVPressureWorkFunctorMaterial.h"
#include "NS.h"
#include "MooseLinearVariableFV.h"

registerMooseObject("NavierStokesApp", NSFVPressureWorkFunctorMaterial);

InputParameters
NSFVPressureWorkFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription(
      "Computes the pressure work density carried by an advecting velocity, dp/dt + u . grad(p). "
      "This is the term an energy equation written in enthalpy carries on its right hand side and "
      "which a weakly compressible formulation drops.");
  params.addRequiredParam<MooseFunctorName>(NS::pressure, "The pressure.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addParam<MooseFunctorName>(
      "pressure_work_name", "pressure_work", "Name to give the computed functor property.");
  params.addParam<bool>("include_time_derivative",
                        true,
                        "Whether to include the transient part, dp/dt, alongside the advective "
                        "part. Set this to false where the term is being assembled in more than "
                        "one piece, so that the transient part is counted once.");
  return params;
}

NSFVPressureWorkFunctorMaterial::NSFVPressureWorkFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _dim(_subproblem.mesh().dimension()),
    _include_time_derivative(getParam<bool>("include_time_derivative")),
    _pressure(getFunctor<Real>(NS::pressure)),
    _u(getFunctor<Real>("u")),
    _v(isParamValid("v") ? &getFunctor<Real>("v") : nullptr),
    _w(isParamValid("w") ? &getFunctor<Real>("w") : nullptr)
{
  if (_dim >= 2 && !_v)
    paramError("v", "In two or more dimensions, the v velocity must be supplied");
  if (_dim >= 3 && !_w)
    paramError("w", "In three dimensions, the w velocity must be supplied");

  // A linear finite volume variable only builds its cell gradients when something asks for them
  // during setup. Kernels do this from their own constructors; a functor material evaluating a
  // gradient has to ask for itself, or the reader it reaches for at evaluation time is not there.
  const auto & pressure_name = getParam<MooseFunctorName>(NS::pressure);
  if (_fe_problem.hasVariable(pressure_name))
    if (auto * const linear_fv_pressure = dynamic_cast<MooseLinearVariableFV<Real> *>(
            &_fe_problem.getVariable(_tid, pressure_name)))
      linear_fv_pressure->requestCellGradients();

  // The transient part comes from the pressure functor's own time derivative, so that it is the
  // same discrete operator the transient terms of the other equations are advanced with rather
  // than a difference formed here. A steady problem returns zero for it.
  const auto pressure_work = [this](const auto & r, const auto & t) -> Real
  {
    RealVectorValue velocity(_u(r, t));
    if (_dim > 1)
      velocity(1) = (*_v)(r, t);
    if (_dim > 2)
      velocity(2) = (*_w)(r, t);

    const auto advective = velocity * _pressure.gradient(r, t);
    return _include_time_derivative ? _pressure.dot(r, t) + advective : advective;
  };

  addFunctorProperty<Real>(getParam<MooseFunctorName>("pressure_work_name"), pressure_work);
}
