//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMFrictionMATRA.h"

#include <cmath>

registerMooseObject("SubChannelApp", SCMFrictionMATRA);

namespace
{
const Real matra_transition_start = std::pow(64.0 / 0.316, 4.0 / 3.0);
const Real matra_transition_end = 5000.0;

Real
smoothStep(const Real x)
{
  return x * x * (3.0 - 2.0 * x);
}

Real
matraQuadLatticeFrictionFactor(const Real Re)
{
  if (Re < 1)
    return 64.0;
  else if (Re < matra_transition_start)
    return 64.0 / Re;
  else if (Re < matra_transition_end)
  {
    const auto weight =
        smoothStep((Re - matra_transition_start) / (matra_transition_end - matra_transition_start));
    const auto laminar_friction = 64.0 / Re;
    const auto matra_friction = 0.316 * std::pow(Re, -0.25);
    return (1.0 - weight) * laminar_friction + weight * matra_friction;
  }
  /// Pang, B. et al. KIT, 2013
  else if (Re >= 5000 and Re < 30000)
    return 0.316 * std::pow(Re, -0.25);
  else if (Re >= 30000 and Re < 1000000)
    return 0.184 * std::pow(Re, -0.20);
  else
    // currently unreachable
    return 0.0;
}
}

InputParameters
SCMFrictionMATRA::validParams()
{
  InputParameters params = SCMFrictionClosureBase::validParams();
  params.addClassDescription(
      "Class that computes the axial friction factor using the MATRA correlation.");
  return params;
}

SCMFrictionMATRA::SCMFrictionMATRA(const InputParameters & parameters)
  : SCMFrictionClosureBase(parameters),
    _is_quad_lattice(dynamic_cast<const QuadSubChannelMesh *>(&_subchannel_mesh) != nullptr),
    _quad_sch_mesh(dynamic_cast<const QuadSubChannelMesh *>(&_subchannel_mesh))
{
}

Real
SCMFrictionMATRA::computeFrictionFactor(const FrictionStruct & friction_args) const
{
  if (_is_quad_lattice)
    return computeQuadLatticeFrictionFactor(friction_args);
  else
    mooseError(name(),
               ": This closure model applies only for assemblies with bare fuel pins in a square "
               "lattice. ");
}

Real
SCMFrictionMATRA::computeQuadLatticeFrictionFactor(const FrictionStruct & friction_args) const
{
  if (friction_args.Re >= 1000000)
  {
    flagInvalidSolution("MATRA correlation out of range");
    return 0.0;
  }

  return matraQuadLatticeFrictionFactor(friction_args.Re);
}
