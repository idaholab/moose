//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalSideJump.h"

#include "Assembly.h"

registerMooseObject("MooseTestApp", InternalSideJump);

InputParameters
InternalSideJump::validParams()
{
  InputParameters params = InternalSidePostprocessor::validParams();
  params.addRequiredCoupledVar("variable", "The name of the variable that this object operates on");
  return params;
}

InternalSideJump::InternalSideJump(const InputParameters & parameters)
  : InternalSidePostprocessor(parameters),
    _current_elem_volume(_assembly.elemVolume()),
    _current_neighbor_volume(_assembly.neighborVolume()),
    _sln_dofs(coupledDofValues("variable")),
    _sln_dofs_neig(coupledNeighborDofValues("variable"))
{
}

void
InternalSideJump::initialize()
{
  _integral_value = 0;
}

void
InternalSideJump::execute()
{
  unsigned int ndofs = _sln_dofs.size();
  Real v = 0;
  for (unsigned int i = 0; i < ndofs; ++i)
    v += _sln_dofs[i];
  v /= ndofs;

  ndofs = _sln_dofs_neig.size();
  Real u = 0;
  for (unsigned int i = 0; i < ndofs; ++i)
    u += _sln_dofs_neig[i];
  u /= ndofs;

  _integral_value += std::abs(u - v) * _current_side_volume;
}

void
InternalSideJump::finalize()
{
  gatherSum(_integral_value);
}

Real
InternalSideJump::getValue()
{
  return _integral_value;
}

void
InternalSideJump::threadJoin(const UserObject & y)
{
  const InternalSideJump & pps = static_cast<const InternalSideJump &>(y);
  _integral_value += pps._integral_value;
}
