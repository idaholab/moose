//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHeatTransferFromHeatStructure3D1PhaseUserObject.h"
#include "THMIndices3Eqn.h"
#include "MooseMesh.h"
#include "KDTree.h"
#include "Assembly.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"

registerMooseObject("ThermalHydraulicsApp", ADHeatTransferFromHeatStructure3D1PhaseUserObject);

InputParameters
ADHeatTransferFromHeatStructure3D1PhaseUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredParam<FlowChannel3DAlignment *>("_fch_alignment",
                                                    "Flow channel alignement object");
  params.addRequiredCoupledVar("P_hf", "Heat flux perimeter");
  params.addRequiredParam<MaterialPropertyName>("T", "Fluid temperature");
  params.addRequiredParam<MaterialPropertyName>("Hw", "Convective heat transfer coefficient");
  params.addClassDescription("Caches heat flux information (fluid temperature and heat transfer "
                             "coefficient) between flow channel and 3D heat structure.");
  return params;
}

ADHeatTransferFromHeatStructure3D1PhaseUserObject::
    ADHeatTransferFromHeatStructure3D1PhaseUserObject(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _fch_alignment(*getParam<FlowChannel3DAlignment *>("_fch_alignment")),
    _P_hf(adCoupledValue("P_hf")),
    _Hw(getADMaterialProperty<Real>("Hw")),
    _T(getADMaterialProperty<Real>("T")),
    _hs_elem_ids(_fch_alignment.getHSElementsIDs())
{

  const auto & hs_boundary_info = _fch_alignment.getHSBoundaryInfo();
  const auto & fch_elem_ids = _fch_alignment.getFlowChannelElementIDs();

  // list of q-points per hs elements
  std::map<dof_id_type, std::vector<Point>> hs_elem_qps;
  // list of q-points per fch elements
  std::map<dof_id_type, std::vector<Point>> fch_elem_qps;

  for (const auto & t : hs_boundary_info)
  {
    auto elem_id = std::get<0>(t);
    auto side_id = std::get<1>(t);
    const Elem * elem = _mesh.elemPtr(elem_id);
    // 2D elements
    _assembly.setCurrentSubdomainID(elem->subdomain_id());
    _assembly.reinit(elem, side_id);
    const MooseArray<Point> & q_points = _assembly.qPointsFace();
    if (q_points.size() == 0)
      mooseError(name(), ": No quadrature points found for element ", elem_id);
    for (std::size_t i = 0; i < q_points.size(); i++)
      hs_elem_qps[elem_id].push_back(q_points[i]);
  }

  for (const auto & elem_id : fch_elem_ids)
  {
    const Elem * elem = _mesh.elemPtr(elem_id);
    // 1D elements
    _assembly.setCurrentSubdomainID(elem->subdomain_id());
    _assembly.reinit(elem);
    const MooseArray<Point> & q_points = _assembly.qPoints();
    if (q_points.size() == 0)
      mooseError("No quadrature points found for element ", elem_id);
    for (std::size_t i = 0; i < q_points.size(); i++)
      fch_elem_qps[elem_id].push_back(q_points[i]);
  }

  // now find out how q-points correspond to each other on the (hs, fch) pair of elements
  for (auto elem_id : _hs_elem_ids)
  {
    dof_id_type nearest_elem_id = _fch_alignment.getNearestElemID(elem_id);
    std::vector<Point> & fch_qps = fch_elem_qps[nearest_elem_id];
    std::vector<Point> & hs_qps = hs_elem_qps[elem_id];
    _elem_qp_map[elem_id].resize(hs_qps.size());
    KDTree kd_tree_qp(fch_qps, _mesh.getMaxLeafSize());
    for (std::size_t i = 0; i < hs_qps.size(); i++)
    {
      unsigned int patch_size = 1;
      std::vector<std::size_t> return_index(patch_size);
      kd_tree_qp.neighborSearch(hs_qps[i], patch_size, return_index);
      _elem_qp_map[elem_id][i] = return_index[0];
    }
  }
}

void
ADHeatTransferFromHeatStructure3D1PhaseUserObject::initialize()
{
  _heated_perimeter.clear();
  _T_fluid.clear();
  _htc.clear();
}

void
ADHeatTransferFromHeatStructure3D1PhaseUserObject::execute()
{
  if (_current_elem->processor_id() == this->processor_id())
  {
    unsigned int n_qpts_fch = _qrule->n_points();
    for (auto elem_id : _hs_elem_ids)
    {
      unsigned int n_qpts_hs = _elem_qp_map[elem_id].size();
      const dof_id_type & nearest_elem_id = _fch_alignment.getNearestElemID(elem_id);
      if (nearest_elem_id == _current_elem->id())
      {
        _heated_perimeter[_current_elem->id()].resize(n_qpts_fch);
        _heated_perimeter[elem_id].resize(n_qpts_hs);

        _T_fluid[_current_elem->id()].resize(n_qpts_fch);
        _T_fluid[elem_id].resize(n_qpts_hs);

        _htc[_current_elem->id()].resize(n_qpts_fch);
        _htc[elem_id].resize(n_qpts_hs);

        for (unsigned int qp_hs = 0; qp_hs < n_qpts_hs; qp_hs++)
        {
          unsigned int nearest_qp = _elem_qp_map[elem_id][qp_hs];

          _T_fluid[_current_elem->id()][nearest_qp] = _T[nearest_qp];
          _T_fluid[elem_id][qp_hs] = _T[nearest_qp];

          _htc[_current_elem->id()][nearest_qp] = _Hw[nearest_qp];
          _htc[elem_id][qp_hs] = _Hw[nearest_qp];

          _heated_perimeter[_current_elem->id()][nearest_qp] = _P_hf[nearest_qp];
          _heated_perimeter[elem_id][qp_hs] = _P_hf[nearest_qp];
        }
      }
    }
  }
}

void
ADHeatTransferFromHeatStructure3D1PhaseUserObject::finalize()
{
  allGatherMap(_heated_perimeter);
  allGatherMap(_T_fluid);
  allGatherMap(_htc);
}

void
ADHeatTransferFromHeatStructure3D1PhaseUserObject::threadJoin(const UserObject & y)
{
  const ADHeatTransferFromHeatStructure3D1PhaseUserObject & uo =
      static_cast<const ADHeatTransferFromHeatStructure3D1PhaseUserObject &>(y);
  for (auto & it : uo._heated_perimeter)
    _heated_perimeter[it.first] = it.second;
  for (auto & it : uo._T_fluid)
    _T_fluid[it.first] = it.second;
  for (auto & it : uo._htc)
    _htc[it.first] = it.second;
}

const std::vector<ADReal> &
ADHeatTransferFromHeatStructure3D1PhaseUserObject::getHeatedPerimeter(dof_id_type element_id) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  auto it = _heated_perimeter.find(element_id);
  if (it != _heated_perimeter.end())
    return it->second;
  else
    mooseError(
        name(), ": Requested heated perimeter for element ", element_id, " was not computed.");
}

const std::vector<ADReal> &
ADHeatTransferFromHeatStructure3D1PhaseUserObject::getHeatTransferCoeff(
    dof_id_type element_id) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  auto it = _htc.find(element_id);
  if (it != _htc.end())
    return it->second;
  else
    mooseError(name(),
               ": Requested heat transfer coefficient for element ",
               element_id,
               " was not computed.");
}

const std::vector<ADReal> &
ADHeatTransferFromHeatStructure3D1PhaseUserObject::getTfluid(dof_id_type element_id) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  auto it = _T_fluid.find(element_id);
  if (it != _T_fluid.end())
    return it->second;
  else
    mooseError(
        name(), ": Requested fluid temperature for element ", element_id, " was not computed.");
}

void
ADHeatTransferFromHeatStructure3D1PhaseUserObject::allGatherMap(
    std::map<dof_id_type, std::vector<ADReal>> & data)
{
  std::vector<std::map<dof_id_type, std::vector<ADReal>>> all;
  comm().allgather(data, all);
  for (auto & hfs : all)
    for (auto & it : hfs)
      data[it.first] = it.second;
}
