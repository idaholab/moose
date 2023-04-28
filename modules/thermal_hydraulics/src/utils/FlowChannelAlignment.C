//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannelAlignment.h"

FlowChannelAlignment::FlowChannelAlignment(const THMMesh & mesh) : MeshAlignment(mesh)
{
  mooseDeprecated("FlowChannelAlignment is deprecated. Use MeshAlignment instead.");
}

void
FlowChannelAlignment::build(
    const std::vector<std::tuple<dof_id_type, unsigned short int>> & master_boundary_info,
    const std::vector<dof_id_type> & slave_elem_ids)
{
  initialize(slave_elem_ids, master_boundary_info);
}

bool
FlowChannelAlignment::check(const std::vector<dof_id_type> & /*fch_elem_ids*/) const
{
  return meshesAreAligned();
}

const dof_id_type &
FlowChannelAlignment::getNearestElemID(const dof_id_type & elem_id) const
{
  auto it = _coupled_elem_ids.find(elem_id);
  if (it != _coupled_elem_ids.end())
    return it->second;
  else
    return DofObject::invalid_id;
}
