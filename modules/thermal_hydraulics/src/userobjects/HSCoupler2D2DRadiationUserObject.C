//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSCoupler2D2DRadiationUserObject.h"
#include "StoreVariableByElemIDSideUserObject.h"
#include "MeshAlignment2D2D.h"
#include "THMUtils.h"
#include "HeatConductionNames.h"

registerMooseObject("ThermalHydraulicsApp", HSCoupler2D2DRadiationUserObject);

InputParameters
HSCoupler2D2DRadiationUserObject::validParams()
{
  InputParameters params = SideUserObject::validParams();

  params.addRequiredParam<std::vector<Real>>("emissivities", "Emissivities of each surface");
  params.addRequiredParam<std::vector<std::vector<Real>>>(
      "view_factors", "View factors between each surface, as a matrix");
  params.addRequiredParam<bool>(
      "include_environment",
      "Whether or not to include an environment surrounding all of the surfaces.");
  params.addParam<Real>("T_environment", 0.0, "Environment temperature.");
  params.addRequiredParam<UserObjectName>(
      "temperature_uo",
      "StoreVariableByElemIDSideUserObjects containing the temperature values for each element");
  params.addRequiredParam<MeshAlignment2D2D *>("mesh_alignment", "Mesh alignment object");

  params.addClassDescription("Computes heat fluxes for HSCoupler2D2D.");

  return params;
}

HSCoupler2D2DRadiationUserObject::HSCoupler2D2DRadiationUserObject(
    const InputParameters & parameters)
  : SideUserObject(parameters),

    _emissivities(getParam<std::vector<Real>>("emissivities")),
    _view_factors(getParam<std::vector<std::vector<Real>>>("view_factors")),
    _include_environment(getParam<bool>("include_environment")),
    _T_environment(getParam<Real>("T_environment")),
    _temperature_uo(getUserObject<StoreVariableByElemIDSideUserObject>("temperature_uo")),
    _mesh_alignment(*getParam<MeshAlignment2D2D *>("mesh_alignment")),
    _n_hs(_emissivities.size()),
    _n_surfaces(_include_environment ? _n_hs + 1 : _n_hs)
{
}

void
HSCoupler2D2DRadiationUserObject::initialize()
{
  _elem_id_to_heat_flux.clear();
}

void
HSCoupler2D2DRadiationUserObject::execute()
{
  // store element IDs corresponding to this side
  std::vector<dof_id_type> elem_ids;
  elem_ids.push_back(_current_elem->id());
  const auto coupled_elem_ids = _mesh_alignment.getCoupledSecondaryElemIDs(elem_ids[0]);
  elem_ids.insert(elem_ids.end(), coupled_elem_ids.begin(), coupled_elem_ids.end());

  // store temperatures
  std::vector<std::vector<ADReal>> temperatures;
  for (const auto & elem_id : elem_ids)
    temperatures.push_back(_temperature_uo.getVariableValues(elem_id));

  // loop over the quadrature points and compute the heat fluxes
  const auto n_qp = _qrule->n_points();
  std::vector<std::vector<ADReal>> heat_fluxes(_n_hs, std::vector<ADReal>(n_qp, 0.0));
  for (unsigned int qp = 0; qp < n_qp; qp++)
  {
    // form matrix and RHS
    DenseMatrix<ADReal> matrix(_n_surfaces, _n_surfaces);
    DenseVector<ADReal> emittances(_n_surfaces);
    for (unsigned int i = 0; i < _n_surfaces; ++i)
    {
      if (_include_environment && i == _n_surfaces - 1) // environment surface
      {
        emittances(i) = HeatConduction::Constants::sigma * std::pow(_T_environment, 4);
        matrix(i, i) = 1.0;
      }
      else // heat structure surface
      {
        emittances(i) =
            _emissivities[i] * HeatConduction::Constants::sigma * std::pow(temperatures[i][qp], 4);
        matrix(i, i) = 1.0;
        for (unsigned int j = 0; j < _n_surfaces; ++j)
          matrix(i, j) -= (1 - _emissivities[i]) * _view_factors[i][j];
      }
    }

    // solve for radiosities
    DenseVector<ADReal> radiosities(_n_surfaces);
    matrix.lu_solve(emittances, radiosities);

    // compute heat fluxes
    for (unsigned int i = 0; i < _n_hs; ++i)
      heat_fluxes[i][qp] =
          1.0 / (1.0 - _emissivities[i]) * (emittances(i) - _emissivities[i] * radiosities(i));
  }

  // store the heat fluxes by element ID
  for (unsigned int i = 0; i < _n_hs; ++i)
    _elem_id_to_heat_flux[elem_ids[i]] = heat_fluxes[i];
}

void
HSCoupler2D2DRadiationUserObject::threadJoin(const UserObject & uo)
{
  const auto & other_uo = static_cast<const HSCoupler2D2DRadiationUserObject &>(uo);
  for (auto & it : other_uo._elem_id_to_heat_flux)
    if (_elem_id_to_heat_flux.find(it.first) == _elem_id_to_heat_flux.end())
      _elem_id_to_heat_flux[it.first] = it.second;
    else
    {
      auto & existing = _elem_id_to_heat_flux[it.first];
      for (const auto qp : index_range(existing))
        existing[qp] += it.second[qp];
    }
}

void
HSCoupler2D2DRadiationUserObject::finalize()
{
  THM::allGatherADVectorMapSum(comm(), _elem_id_to_heat_flux);
}

const std::vector<ADReal> &
HSCoupler2D2DRadiationUserObject::getHeatFlux(dof_id_type elem_id) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  auto it = _elem_id_to_heat_flux.find(elem_id);
  if (it != _elem_id_to_heat_flux.end())
    return it->second;
  else
    mooseError(name(), ": Requested heat flux for element ", elem_id, " was not computed.");
}
