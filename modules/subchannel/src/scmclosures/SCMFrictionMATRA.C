//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMFrictionMATRA.h"

registerMooseObject("SubChannelApp", SCMFrictionMATRA);

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
  auto Re = friction_args.Re;
  Real a(0.0), b(0.0);
  if (Re < 1)
  {
    a = 64.0;
    b = 0.0;
  }
  else if (Re >= 1 and Re < 5000)
  {
    a = 64.0;
    b = -1.0;
  }
  /// Pang, B. et al. KIT, 2013
  else if (Re >= 5000 and Re < 30000)
  {
    a = 0.316;
    b = -0.25;
  }
  else if (Re >= 30000 and Re < 1000000)
  {
    a = 0.184;
    b = -0.20;
  }
  else
  {
    flagInvalidSolution("MATRA correlation out of range");
  }
  return a * std::pow(Re, b);
}
