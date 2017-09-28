/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "InternalSideJump.h"

#include "Assembly.h"

template <>
InputParameters
validParams<InternalSideJump>()
{
  InputParameters params = validParams<InternalSidePostprocessor>();
  params.addRequiredCoupledVar("variable", "The name of the variable that this object operates on");
  return params;
}

InternalSideJump::InternalSideJump(const InputParameters & parameters)
  : InternalSidePostprocessor(parameters),
    MooseVariableInterface(this, false),
    _current_elem_volume(_assembly.elemVolume()),
    _current_neighbor_volume(_assembly.neighborVolume()),
    _sln_dofs(coupledSolutionDoFs("variable")),
    _sln_dofs_neig(coupledNeighborSolutionDoFs("variable"))
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
    v += _sln_dofs(i);
  v /= ndofs;

  ndofs = _sln_dofs_neig.size();
  Real u = 0;
  for (unsigned int i = 0; i < ndofs; ++i)
    u += _sln_dofs_neig(i);
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
