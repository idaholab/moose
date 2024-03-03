//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalWaveSpeed.h"

// MOOSE includes
#include "MooseVariable.h"
#include "SystemBase.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/quadrature.h"

registerMooseObject("ContactApp", NodalWaveSpeed);

InputParameters
NodalWaveSpeed::validParams()
{
  InputParameters params = SideIntegralVariableUserObject::validParams();
  params.set<ExecFlagEnum>("execute_on") = EXEC_LINEAR;
  params.addClassDescription("Compute the tributary wave speeds for nodes on a surface");
  return params;
}

NodalWaveSpeed::NodalWaveSpeed(const InputParameters & parameters)
  : SideIntegralVariableUserObject(parameters),
    _phi(_variable->phiFace()),
    _system(_variable->sys()),
    _aux_solution(_system.solution()),
    _wave_speed(getMaterialProperty<Real>("wave_speed"))
{
}

NodalWaveSpeed::~NodalWaveSpeed() {}

void
NodalWaveSpeed::threadJoin(const UserObject & fred)
{
  const NodalWaveSpeed & na = dynamic_cast<const NodalWaveSpeed &>(fred);

  std::map<const Node *, Real>::const_iterator it = na._node_wave_speeds.begin();
  const std::map<const Node *, Real>::const_iterator it_end = na._node_wave_speeds.end();
  for (; it != it_end; ++it)
    _node_wave_speeds[it->first] += it->second;
}

void
NodalWaveSpeed::initialize()
{
  _node_wave_speeds.clear();
}

void
NodalWaveSpeed::execute()
{
  std::vector<Real> node_wave_speeds(_phi.size());
  for (unsigned qp(0); qp < _qrule->n_points(); ++qp)
    for (unsigned j(0); j < _phi.size(); ++j)
      node_wave_speeds[j] += (_phi[j][qp] * _wave_speed[qp]);

  for (unsigned j(0); j < _phi.size(); ++j)
  {
    const Real wave_speed = node_wave_speeds[j];
    if (wave_speed != 0)
      _node_wave_speeds[_current_elem->node_ptr(j)] = wave_speed;
  }
}

void
NodalWaveSpeed::finalize()
{
  const std::map<const Node *, Real>::iterator it_end = _node_wave_speeds.end();
  for (std::map<const Node *, Real>::iterator it = _node_wave_speeds.begin(); it != it_end; ++it)
  {
    const Node * const node = it->first;
    dof_id_type dof = node->dof_number(_system.number(), _variable->number(), 0);
    _aux_solution.set(dof, it->second);
  }
  _aux_solution.close();
}
