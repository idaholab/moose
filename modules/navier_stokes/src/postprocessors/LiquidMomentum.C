//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LiquidMomentum.h"

#include "MooseMesh.h"
#include "metaphysicl/raw_type.h"

#include <algorithm>

registerMooseObject("NavierStokesApp", LiquidMomentum);

namespace
{
Real
clampPhaseFraction(const Real value)
{
  return std::max(0.0, std::min(1.0, value));
}
}

InputParameters
LiquidMomentum::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<MooseFunctorName>("volume_fraction",
                                            "The liquid volume-fraction functor.");
  params.addRequiredParam<MooseFunctorName>("liquid_density",
                                            "The liquid density functor used in the momentum.");
  params.addRequiredParam<MooseFunctorName>("velocity",
                                            "The velocity component functor used in the momentum.");
  params.addClassDescription("Computes the integrated liquid momentum for one velocity component.");
  return params;
}

LiquidMomentum::LiquidMomentum(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _volume_fraction(getFunctor<Real>("volume_fraction")),
    _liquid_density(getFunctor<Real>("liquid_density")),
    _velocity_component(getFunctor<Real>("velocity")),
    _momentum(0.0)
{
}

void
LiquidMomentum::initialize()
{
  _momentum = 0.0;
}

void
LiquidMomentum::execute()
{
  const auto time_arg = determineState();

  for (const auto & elem : _mesh.getMesh().active_local_element_ptr_range())
  {
    const auto elem_arg = makeElemArg(elem);
    const Real alpha = clampPhaseFraction(MetaPhysicL::raw_value(_volume_fraction(elem_arg, time_arg)));
    const Real liquid_density = MetaPhysicL::raw_value(_liquid_density(elem_arg, time_arg));
    const Real velocity = MetaPhysicL::raw_value(_velocity_component(elem_arg, time_arg));
    _momentum += alpha * liquid_density * velocity * elem->volume();
  }
}

void
LiquidMomentum::finalize()
{
  _communicator.sum(_momentum);
}

Real
LiquidMomentum::getValue() const
{
  return _momentum;
}
