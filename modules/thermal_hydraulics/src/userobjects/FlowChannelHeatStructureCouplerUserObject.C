//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannelHeatStructureCouplerUserObject.h"
#include "MooseMesh.h"
#include "KDTree.h"
#include "Assembly.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"

InputParameters
FlowChannelHeatStructureCouplerUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredParam<FlowChannelAlignment *>("_fch_alignment",
                                                  "Flow channel alignment object");
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
    _fch_alignment(*getParam<FlowChannelAlignment *>("_fch_alignment")),
    _elem_qp_map(buildQuadraturePointMap())
{
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
  if (_current_elem->processor_id() == this->processor_id())
  {
    _fc_elem_id = _current_elem->id();
    _hs_elem_id = _fch_alignment.getNearestElemID(_current_elem->id());

    const auto map_ptrs = getCachedQuantityMaps();
    for (const auto map_ptr : map_ptrs)
    {
      (*map_ptr)[_fc_elem_id].resize(n_qpts);
      (*map_ptr)[_hs_elem_id].resize(n_qpts);
    }

    for (_fc_qp = 0; _fc_qp < n_qpts; _fc_qp++)
    {
      _hs_qp = _elem_qp_map.at(_fc_elem_id)[_fc_qp];
      computeQpCachedQuantities();
    }
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

std::map<dof_id_type, std::vector<unsigned int>>
FlowChannelHeatStructureCouplerUserObject::buildQuadraturePointMap() const
{
  const auto & hs_boundary_info = _fch_alignment.getMasterBoundaryInfo();
  const auto & fc_elem_ids = _fch_alignment.getSlaveElementIDs();

  // build list of q-points for each heat structure element
  std::map<dof_id_type, std::vector<Point>> hs_elem_qps;
  for (const auto & elem_and_side : hs_boundary_info)
  {
    auto elem_id = std::get<0>(elem_and_side);
    auto side_id = std::get<1>(elem_and_side);
    const Elem * elem = _mesh.elemPtr(elem_id);
    _assembly.setCurrentSubdomainID(elem->subdomain_id());
    _assembly.reinit(elem, side_id);
    const MooseArray<Point> & q_points = _assembly.qPointsFace();
    for (std::size_t i = 0; i < q_points.size(); i++)
      hs_elem_qps[elem_id].push_back(q_points[i]);
  }

  // build list of q-points for each flow channel element
  std::map<dof_id_type, std::vector<Point>> fc_elem_qps;
  for (const auto & elem_id : fc_elem_ids)
  {
    const Elem * elem = _mesh.elemPtr(elem_id);
    _assembly.setCurrentSubdomainID(elem->subdomain_id());
    _assembly.reinit(elem);
    const MooseArray<Point> & q_points = _assembly.qPoints();
    for (std::size_t i = 0; i < q_points.size(); i++)
      fc_elem_qps[elem_id].push_back(q_points[i]);
  }

  // build mapping
  std::map<dof_id_type, std::vector<unsigned int>> elem_qp_map;
  for (auto elem_id : fc_elem_ids)
  {
    dof_id_type nearest_elem_id = _fch_alignment.getNearestElemID(elem_id);

    std::vector<Point> & fc_qps = fc_elem_qps[elem_id];
    std::vector<Point> & hs_qps = hs_elem_qps[nearest_elem_id];
    elem_qp_map[elem_id].resize(fc_qps.size());
    KDTree kd_tree_qp(hs_qps, 5);
    for (std::size_t i = 0; i < fc_qps.size(); i++)
    {
      unsigned int patch_size = 1;
      std::vector<std::size_t> return_index(patch_size);
      kd_tree_qp.neighborSearch(fc_qps[i], patch_size, return_index);
      elem_qp_map[elem_id][i] = return_index[0];
    }
  }

  return elem_qp_map;
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
