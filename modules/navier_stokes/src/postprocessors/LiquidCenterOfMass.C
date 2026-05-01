//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LiquidCenterOfMass.h"

#include "MooseMesh.h"
#include "metaphysicl/raw_type.h"

#include <algorithm>

registerMooseObject("NavierStokesApp", LiquidCenterOfMass);

namespace
{
Real
clampLiquidFraction(const Real value)
{
  return std::max(0.0, std::min(1.0, value));
}
}

InputParameters
LiquidCenterOfMass::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum direction("x=0 y=1 z=2", "x");
  params.addRequiredParam<MooseFunctorName>("volume_fraction",
                                            "The liquid volume-fraction functor.");
  params.addRequiredParam<MooseFunctorName>("liquid_density",
                                            "The liquid density functor used for mass weighting.");
  params.addParam<MooseEnum>(
      "direction", direction, "Coordinate direction of the reported liquid center of mass.");
  params.addParam<Real>("fallback_value",
                        0.0,
                        "Value returned if the liquid phase has zero integrated mass.");
  params.addClassDescription("Computes a mass-weighted liquid center-of-mass coordinate.");
  return params;
}

LiquidCenterOfMass::LiquidCenterOfMass(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _volume_fraction(getFunctor<Real>("volume_fraction")),
    _liquid_density(getFunctor<Real>("liquid_density")),
    _direction(static_cast<unsigned int>(getParam<MooseEnum>("direction"))),
    _fallback_value(getParam<Real>("fallback_value")),
    _weighted_coordinate_sum(0.0),
    _mass_sum(0.0),
    _value(_fallback_value)
{
  if (_direction >= _mesh.dimension())
    paramError("direction",
               "Requested direction is incompatible with mesh dimension ",
               _mesh.dimension(),
               ".");
}

void
LiquidCenterOfMass::initialize()
{
  _weighted_coordinate_sum = 0.0;
  _mass_sum = 0.0;
}

void
LiquidCenterOfMass::execute()
{
  const auto time_arg = determineState();

  for (const auto & elem : _mesh.getMesh().active_local_element_ptr_range())
  {
    const auto elem_arg = makeElemArg(elem);
    const Real alpha = clampLiquidFraction(MetaPhysicL::raw_value(_volume_fraction(elem_arg, time_arg)));
    const Real liquid_density = MetaPhysicL::raw_value(_liquid_density(elem_arg, time_arg));
    const Real liquid_mass = alpha * liquid_density * elem->volume();
    const Point centroid = elem->vertex_average();

    _mass_sum += liquid_mass;
    _weighted_coordinate_sum += liquid_mass * centroid(_direction);
  }
}

void
LiquidCenterOfMass::finalize()
{
  _communicator.sum(_mass_sum);
  _communicator.sum(_weighted_coordinate_sum);

  _value =
      std::abs(_mass_sum) > libMesh::TOLERANCE ? _weighted_coordinate_sum / _mass_sum : _fallback_value;
}

Real
LiquidCenterOfMass::getValue() const
{
  return _value;
}
