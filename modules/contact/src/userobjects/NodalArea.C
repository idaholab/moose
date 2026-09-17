//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalArea.h"

// MOOSE includes
#include "MooseVariable.h"
#include "SystemBase.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/quadrature.h"

registerMooseObject("ContactApp", NodalArea);

InputParameters
NodalArea::validParams()
{
  InputParameters params = SideIntegralVariableUserObject::validParams();
  params.set<ExecFlagEnum>("execute_on") = EXEC_LINEAR;
  params.addClassDescription("Compute the tributary area for nodes on a surface");
  return params;
}

NodalArea::NodalArea(const InputParameters & parameters)
  : SideIntegralVariableUserObject(parameters),
    _phi(_variable->phiFace()),
    _system(_variable->sys()),
    _aux_solution(_system.solution())
{
}

NodalArea::~NodalArea() {}

void
NodalArea::threadJoin(const UserObject & fred)
{
  const auto & na = static_cast<const NodalArea &>(fred);

  std::map<const Node *, Real>::const_iterator it = na._node_areas.begin();
  const std::map<const Node *, Real>::const_iterator it_end = na._node_areas.end();
  for (; it != it_end; ++it)
  {
    _node_areas[it->first] += it->second;
  }
}

Real
NodalArea::computeQpIntegral()
{
  return 1;
}

void
NodalArea::initialize()
{
  _node_areas.clear();
}

void
NodalArea::execute()
{
  std::vector<Real> nodeAreas(_phi.size());
  for (unsigned qp(0); qp < _qrule->n_points(); ++qp)
  {
    for (unsigned j(0); j < _phi.size(); ++j)
    {
      nodeAreas[j] += (_phi[j][qp] * _JxW[qp] * _coord[qp]);
    }
  }
  for (unsigned j(0); j < _phi.size(); ++j)
  {
    const Real area = nodeAreas[j];
    if (area != 0)
    {
      _node_areas[_current_elem->node_ptr(j)] += area;
    }
  }
}

void
NodalArea::finalize()
{

  const std::map<const Node *, Real>::iterator it_end = _node_areas.end();
  for (std::map<const Node *, Real>::iterator it = _node_areas.begin(); it != it_end; ++it)
  {
    const Node * const node = it->first;
    dof_id_type dof = node->dof_number(_system.number(), _variable->number(), 0);
    _aux_solution.set(dof, 0);
  }
  _aux_solution.close();

  for (std::map<const Node *, Real>::iterator it = _node_areas.begin(); it != it_end; ++it)
  {
    const Node * const node = it->first;
    dof_id_type dof = node->dof_number(_system.number(), _variable->number(), 0);
    _aux_solution.add(dof, it->second);
  }
  _aux_solution.close();
}

Real
NodalArea::nodalArea(const Node * node) const
{
  // Read the tributary area from the associated aux variable's ghosted
  // solution vector -- that vector is populated by `finalize()` (see
  // above) and is properly ghosted, so this returns the correct value
  // for both locally-owned and ghosted nodes.  The prior implementation
  // read from the `_node_areas` map which only holds LOCAL
  // contributions and returns 0 for nodes owned by other ranks; that
  // caused per-node weights to be zeroed out for consumers that touch
  // the entire boundary from a single rank (e.g.
  // RigidBodyLoadControl's ScalarKernel, which is called only on the
  // scalar-owning rank).
  if (node->n_dofs(_system.number(), _variable->number()) == 0)
    return 0;
  const dof_id_type dof = node->dof_number(_system.number(), _variable->number(), 0);
  // Read from the GHOSTED aux solution (currentSolution), not the
  // parallel `_aux_solution` -- for a non-local dof, calling operator()
  // on the non-ghosted vector is undefined behaviour and can segfault
  // on the scalar-owning rank of RigidBodyLoadControl.
  const auto * cs = _system.currentSolution();
  return cs ? (*cs)(dof) : _aux_solution(dof);
}
