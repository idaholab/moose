//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatFluxFromHeatStructureBaseUserObject.h"
#include "MooseMesh.h"
#include "KDTree.h"
#include "Assembly.h"

InputParameters
HeatFluxFromHeatStructureBaseUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredParam<FlowChannelAlignment *>("_fch_alignment",
                                                  "Flow channel alignement object");
  params.addRequiredCoupledVar("P_hf", "Heat flux perimeter");
  params.addClassDescription(
      "Base class for caching heat flux between flow channels and heat structures.");
  return params;
}

HeatFluxFromHeatStructureBaseUserObject::HeatFluxFromHeatStructureBaseUserObject(
    const InputParameters & parameters)
  : ElementUserObject(parameters),
    _fch_alignment(*getParam<FlowChannelAlignment *>("_fch_alignment")),
    _P_hf(coupledValue("P_hf"))
{
  const auto & master_boundary_info = _fch_alignment.getMasterBoundaryInfo();
  const auto & slave_elem_ids = _fch_alignment.getSlaveElementIDs();

  // list of q-points per master elements
  std::map<dof_id_type, std::vector<Point>> master_elem_qps;
  // list of q-points per slave elements
  std::map<dof_id_type, std::vector<Point>> slave_elem_qps;

  for (const auto & t : master_boundary_info)
  {
    auto elem_id = std::get<0>(t);
    auto side_id = std::get<1>(t);
    const Elem * elem = _mesh.elemPtr(elem_id);
    if (elem->processor_id() == _subproblem.processor_id())
    {
      // 2D elements
      _assembly.setCurrentSubdomainID(elem->subdomain_id());
      _assembly.reinit(elem, side_id);
      const MooseArray<Point> & q_points = _assembly.qPointsFace();
      for (std::size_t i = 0; i < q_points.size(); i++)
        master_elem_qps[elem_id].push_back(q_points[i]);
    }
  }

  for (const auto & elem_id : slave_elem_ids)
  {
    const Elem * elem = _mesh.elemPtr(elem_id);
    if (elem->processor_id() == _subproblem.processor_id())
    {
      // 1D elements
      _assembly.setCurrentSubdomainID(elem->subdomain_id());
      _assembly.reinit(elem);
      const MooseArray<Point> & q_points = _assembly.qPoints();
      for (std::size_t i = 0; i < q_points.size(); i++)
        slave_elem_qps[elem_id].push_back(q_points[i]);
    }
  }

  // now find out how q-points correspond to each other on the (master, slave) pair of elements
  for (auto elem_id : slave_elem_ids)
  {
    dof_id_type nearest_elem_id = _fch_alignment.getNearestElemID(elem_id);

    std::vector<Point> & slave_qps = slave_elem_qps[elem_id];
    std::vector<Point> & master_qps = master_elem_qps[nearest_elem_id];
    mooseAssert(slave_qps.size() == master_qps.size(),
                "Number of master and slave q-points have to match");
    _elem_qp_map[elem_id].resize(slave_qps.size());
    KDTree kd_tree_qp(master_qps, 5);
    for (std::size_t i = 0; i < slave_qps.size(); i++)
    {
      unsigned int patch_size = 1;
      std::vector<std::size_t> return_index(patch_size);
      kd_tree_qp.neighborSearch(slave_qps[i], patch_size, return_index);
      _elem_qp_map[elem_id][i] = return_index[0];
    }
  }
}

void
HeatFluxFromHeatStructureBaseUserObject::initialize()
{
}

void
HeatFluxFromHeatStructureBaseUserObject::execute()
{
  unsigned int n_qpts = _qrule->n_points();
  const dof_id_type & nearest_elem_id = _fch_alignment.getNearestElemID(_current_elem->id());

  _heated_perimeter[_current_elem->id()].resize(n_qpts);
  _heated_perimeter[nearest_elem_id].resize(n_qpts);

  _heat_flux[_current_elem->id()].resize(n_qpts);
  _heat_flux[nearest_elem_id].resize(n_qpts);
  for (_qp = 0; _qp < n_qpts; _qp++)
  {
    unsigned int nearest_qp = _elem_qp_map[_current_elem->id()][_qp];
    Real q_wall = computeQpHeatFlux();

    _heat_flux[_current_elem->id()][_qp] = q_wall;
    _heat_flux[nearest_elem_id][nearest_qp] = q_wall;

    _heated_perimeter[_current_elem->id()][_qp] = _P_hf[_qp];
    _heated_perimeter[nearest_elem_id][nearest_qp] = _P_hf[_qp];
  }

  if (_fe_problem.currentlyComputingJacobian())
  {
    _heat_flux_jacobian[_current_elem->id()].resize(n_qpts);
    _heat_flux_jacobian[nearest_elem_id].resize(n_qpts);
    for (_qp = 0; _qp < n_qpts; _qp++)
    {
      unsigned int nearest_qp = _elem_qp_map[_current_elem->id()][_qp];
      DenseVector<Real> jac = computeQpHeatFluxJacobian();

      _heat_flux_jacobian[_current_elem->id()][_qp] = jac;
      _heat_flux_jacobian[nearest_elem_id][nearest_qp] = jac;
    }
  }
}

void
HeatFluxFromHeatStructureBaseUserObject::finalize()
{
}

void
HeatFluxFromHeatStructureBaseUserObject::threadJoin(const UserObject & y)
{
  const HeatFluxFromHeatStructureBaseUserObject & uo =
      static_cast<const HeatFluxFromHeatStructureBaseUserObject &>(y);
  for (auto & it : uo._heated_perimeter)
    _heated_perimeter[it.first] = it.second;
  for (auto & it : uo._heat_flux)
    _heat_flux[it.first] = it.second;
  for (auto & it : uo._heat_flux_jacobian)
    _heat_flux_jacobian[it.first] = it.second;
}

const std::vector<Real> &
HeatFluxFromHeatStructureBaseUserObject::getHeatedPerimeter(dof_id_type element_id) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  auto it = _heated_perimeter.find(element_id);
  if (it != _heated_perimeter.end())
    return it->second;
  else
    mooseError(
        name(), ": Requested heated perimeter for element ", element_id, " was not computed.");
}

const std::vector<Real> &
HeatFluxFromHeatStructureBaseUserObject::getHeatFlux(dof_id_type element_id) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  auto it = _heat_flux.find(element_id);
  if (it != _heat_flux.end())
    return it->second;
  else
    mooseError(name(), ": Requested heat flux for element ", element_id, " was not computed.");
}

const std::vector<DenseVector<Real>> &
HeatFluxFromHeatStructureBaseUserObject::getHeatFluxJacobian(dof_id_type element_id) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  auto it = _heat_flux_jacobian.find(element_id);
  if (it != _heat_flux_jacobian.end())
    return it->second;
  else
    mooseError(
        name(), ": Requested heat flux jacobian for element ", element_id, " was not computed.");
}
