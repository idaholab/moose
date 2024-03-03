//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalDensity.h"

// MOOSE includes
#include "MooseVariable.h"
#include "SystemBase.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/quadrature.h"

registerMooseObject("ContactApp", NodalDensity);

InputParameters
NodalDensity::validParams()
{
  InputParameters params = SideIntegralVariableUserObject::validParams();
  params.set<ExecFlagEnum>("execute_on") = EXEC_LINEAR;
  params.addClassDescription("Compute the tributary densities for nodes on a surface");
  return params;
}

NodalDensity::NodalDensity(const InputParameters & parameters)
  : SideIntegralVariableUserObject(parameters),
    _phi(_variable->phiFace()),
    _system(_variable->sys()),
    _aux_solution(_system.solution()),
    _density(getMaterialProperty<Real>("density"))
{
}

NodalDensity::~NodalDensity() {}

void
NodalDensity::threadJoin(const UserObject & fred)
{
  const NodalDensity & na = dynamic_cast<const NodalDensity &>(fred);

  std::map<const Node *, Real>::const_iterator it = na._node_densities.begin();
  const std::map<const Node *, Real>::const_iterator it_end = na._node_densities.end();
  for (; it != it_end; ++it)
  {
    _node_densities[it->first] += it->second;
  }
}

void
NodalDensity::initialize()
{
  _node_densities.clear();
}

void
NodalDensity::execute()
{
  std::vector<Real> node_densities(_phi.size());
  for (unsigned qp(0); qp < _qrule->n_points(); ++qp)
    for (unsigned j(0); j < _phi.size(); ++j)
      node_densities[j] += (_phi[j][qp] * _density[qp]);

  for (unsigned j(0); j < _phi.size(); ++j)
  {
    const Real density = node_densities[j];
    if (density != 0)
      _node_densities[_current_elem->node_ptr(j)] = density;
  }
}

void
NodalDensity::finalize()
{

  const std::map<const Node *, Real>::iterator it_end = _node_densities.end();
  for (std::map<const Node *, Real>::iterator it = _node_densities.begin(); it != it_end; ++it)
  {
    const Node * const node = it->first;
    dof_id_type dof = node->dof_number(_system.number(), _variable->number(), 0);
    _aux_solution.set(dof, it->second);
  }
  _aux_solution.close();
}
