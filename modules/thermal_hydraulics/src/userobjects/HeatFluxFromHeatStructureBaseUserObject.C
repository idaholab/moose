//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatFluxFromHeatStructureBaseUserObject.h"

InputParameters
HeatFluxFromHeatStructureBaseUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredParam<MeshAlignment *>("_mesh_alignment", "Mesh alignment object");
  params.addRequiredCoupledVar("P_hf", "Heat flux perimeter");
  params.addClassDescription(
      "Base class for caching heat flux between flow channels and heat structures.");
  return params;
}

HeatFluxFromHeatStructureBaseUserObject::HeatFluxFromHeatStructureBaseUserObject(
    const InputParameters & parameters)
  : ElementUserObject(parameters),
    _mesh_alignment(*getParam<MeshAlignment *>("_mesh_alignment")),
    _P_hf(coupledValue("P_hf"))
{
  _mesh_alignment.buildCoupledElemQpIndexMap(_assembly);
}

void
HeatFluxFromHeatStructureBaseUserObject::initialize()
{
}

void
HeatFluxFromHeatStructureBaseUserObject::execute()
{
  unsigned int n_qpts = _qrule->n_points();
  const dof_id_type & nearest_elem_id = _mesh_alignment.getCoupledElemID(_current_elem->id());

  _heated_perimeter[_current_elem->id()].resize(n_qpts);
  _heated_perimeter[nearest_elem_id].resize(n_qpts);

  _heat_flux[_current_elem->id()].resize(n_qpts);
  _heat_flux[nearest_elem_id].resize(n_qpts);
  for (_qp = 0; _qp < n_qpts; _qp++)
  {
    unsigned int nearest_qp = _mesh_alignment.getCoupledElemQpIndex(_current_elem->id(), _qp);
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
      unsigned int nearest_qp = _mesh_alignment.getCoupledElemQpIndex(_current_elem->id(), _qp);
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
