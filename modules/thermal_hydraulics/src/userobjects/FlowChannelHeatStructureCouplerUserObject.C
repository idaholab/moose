//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannelHeatStructureCouplerUserObject.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"

InputParameters
FlowChannelHeatStructureCouplerUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredParam<MeshAlignment *>("_mesh_alignment", "Mesh alignment object");
  params.addClassDescription(
      "Base class for caching quantities computed between flow channels and heat structures.");
  return params;
}

FlowChannelHeatStructureCouplerUserObject::FlowChannelHeatStructureCouplerUserObject(
    const InputParameters & parameters)
  : ElementUserObject(parameters),
    _fc_elem_id(0),
    _hs_elem_id(0),
    _fc_qp(0),
    _hs_qp(0),
    _mesh_alignment(*getParam<MeshAlignment *>("_mesh_alignment"))
{
  _mesh_alignment.buildCoupledElemQpIndexMap(_assembly);
}

void
FlowChannelHeatStructureCouplerUserObject::initialize()
{
  const auto map_ptrs = getCachedQuantityMaps();
  for (const auto map_ptr : map_ptrs)
    map_ptr->clear();
}

void
FlowChannelHeatStructureCouplerUserObject::execute()
{
  unsigned int n_qpts = _qrule->n_points();

  _fc_elem_id = _current_elem->id();
  _hs_elem_id = _mesh_alignment.getCoupledElemID(_fc_elem_id);

  const auto map_ptrs = getCachedQuantityMaps();
  for (const auto map_ptr : map_ptrs)
  {
    (*map_ptr)[_fc_elem_id].resize(n_qpts);
    (*map_ptr)[_hs_elem_id].resize(n_qpts);
  }

  for (_fc_qp = 0; _fc_qp < n_qpts; _fc_qp++)
  {
    _hs_qp = _mesh_alignment.getCoupledElemQpIndex(_fc_elem_id, _fc_qp);
    computeQpCachedQuantities();
  }
}

void
FlowChannelHeatStructureCouplerUserObject::threadJoin(const UserObject & uo)
{
  const auto & fc_hs_uo = static_cast<const FlowChannelHeatStructureCouplerUserObject &>(uo);

  const auto map_ptrs = getCachedQuantityMaps();
  const auto other_map_ptrs = fc_hs_uo.getCachedQuantityMaps();

  for (unsigned int i = 0; i < map_ptrs.size(); ++i)
    for (auto & it : *other_map_ptrs[i])
      (*map_ptrs[i])[it.first] = it.second;
}

void
FlowChannelHeatStructureCouplerUserObject::finalize()
{
  const auto map_ptrs = getCachedQuantityMaps();
  for (const auto map_ptr : map_ptrs)
    allGatherMap(*map_ptr);
}

const std::vector<ADReal> &
FlowChannelHeatStructureCouplerUserObject::getCachedQuantity(
    dof_id_type elem_id,
    const std::map<dof_id_type, std::vector<ADReal>> & elem_id_to_values,
    const std::string & description) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  auto it = elem_id_to_values.find(elem_id);
  if (it != elem_id_to_values.end())
    return it->second;
  else
    mooseError(name(),
               ": The ",
               description,
               " for element ",
               elem_id,
               " was requested but not computed.");
}

void
FlowChannelHeatStructureCouplerUserObject::allGatherMap(
    std::map<dof_id_type, std::vector<ADReal>> & data)
{
  std::vector<std::map<dof_id_type, std::vector<ADReal>>> all;
  comm().allgather(data, all);
  for (auto & hfs : all)
    for (auto & it : hfs)
      data[it.first] = it.second;
}
