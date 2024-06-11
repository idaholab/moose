//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HSCoupler2D3DUserObject.h"
#include "StoreVariableByElemIDSideUserObject.h"
#include "MeshAlignment2D3D.h"
#include "THMUtils.h"
#include "HeatTransferModels.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", HSCoupler2D3DUserObject);

InputParameters
HSCoupler2D3DUserObject::validParams()
{
  InputParameters params = SideUserObject::validParams();

  params.addRequiredCoupledVar("temperature", "Temperature");
  params.addRequiredParam<Real>("radius_2d", "Radius of the 2D heat structure boundary [m]");
  params.addParam<FunctionName>(
      "emissivity_2d",
      0,
      "Emissivity of the 2D heat structure boundary as a function of temperature [K]");
  params.addParam<FunctionName>(
      "emissivity_3d",
      0,
      "Emissivity of the 3D heat structure boundary as a function of temperature [K]");
  params.addRequiredParam<FunctionName>("gap_thickness",
                                        "Gap thickness [m] as a function of temperature [K]");
  params.addRequiredParam<FunctionName>(
      "gap_thermal_conductivity",
      "Gap thermal conductivity [W/(m-K)] as a function of temperature [K]");
  params.addParam<FunctionName>(
      "gap_htc", 0, "Gap heat transfer coefficient [W/(m^2-K)] as a function of temperature [K]");
  params.addRequiredParam<UserObjectName>(
      "temperature_2d_uo",
      "StoreVariableByElemIDSideUserObject containing the temperature values on the 2D boundary");
  params.addRequiredParam<MeshAlignment2D3D *>("mesh_alignment", "Mesh alignment object");

  params.addClassDescription("Computes heat fluxes for HSCoupler2D3D.");

  return params;
}

HSCoupler2D3DUserObject::HSCoupler2D3DUserObject(const InputParameters & parameters)
  : SideUserObject(parameters),

    _T_3d(adCoupledValue("temperature")),
    _r_2d(getParam<Real>("radius_2d")),
    _emissivity_2d_fn(getFunction("emissivity_2d")),
    _emissivity_3d_fn(getFunction("emissivity_3d")),
    _include_radiation(isParamSetByUser("emissivity_2d") && isParamSetByUser("emissivity_3d")),
    _gap_thickness_fn(getFunction("gap_thickness")),
    _k_gap_fn(getFunction("gap_thermal_conductivity")),
    _htc_gap_fn(getFunction("gap_htc")),
    _temperature_2d_uo(getUserObject<StoreVariableByElemIDSideUserObject>("temperature_2d_uo")),
    _mesh_alignment(*getParam<MeshAlignment2D3D *>("mesh_alignment"))
{
  _mesh_alignment.buildCoupledElemQpIndexMapSecondary(_assembly);
}

void
HSCoupler2D3DUserObject::initialize()
{
  _elem_id_to_heat_flux.clear();
}

void
HSCoupler2D3DUserObject::execute()
{
  const auto elem_id_3d = _current_elem->id();
  const auto elem_id_2d = _mesh_alignment.getCoupledPrimaryElemID(elem_id_3d);

  const auto n_qp_2d = _mesh_alignment.getPrimaryNumberOfQuadraturePoints();
  const auto n_qp_3d = _qrule->n_points();

  const auto & T_2d_values = _temperature_2d_uo.getVariableValues(elem_id_2d);
  const auto & area_2d = _mesh_alignment.getPrimaryArea(elem_id_2d);

  std::vector<ADReal> heat_flux_3d(n_qp_3d, 0.0);
  std::vector<ADReal> heat_flux_2d(n_qp_2d, 0.0);
  for (unsigned int qp_3d = 0; qp_3d < n_qp_3d; qp_3d++)
  {
    const auto qp_2d = _mesh_alignment.getCoupledPrimaryElemQpIndex(elem_id_3d, qp_3d);

    const auto & T_2d = T_2d_values[qp_2d];
    const auto & T_3d = _T_3d[qp_3d];
    const auto T_gap = 0.5 * (T_2d + T_3d);

    const auto gap_thickness = evaluateTemperatureFunction(_gap_thickness_fn, T_gap);
    const auto k_gap = evaluateTemperatureFunction(_k_gap_fn, T_gap);

    if (!MooseUtils::absoluteFuzzyGreaterThan(gap_thickness, 0))
      mooseError("Gap thickness must be > 0.");

    const auto r_3d = _r_2d + gap_thickness;

    const auto heat_flux_cond =
        HeatTransferModels::cylindricalGapConductionHeatFlux(k_gap, _r_2d, r_3d, T_2d, T_3d);
    auto heat_flux = heat_flux_cond;

    const auto htc = evaluateTemperatureFunction(_htc_gap_fn, T_gap);
    heat_flux += htc * (T_2d - T_3d);

    if (_include_radiation)
    {
      const auto emissivity_2d = evaluateTemperatureFunction(_emissivity_2d_fn, T_2d);
      const auto emissivity_3d = evaluateTemperatureFunction(_emissivity_3d_fn, T_3d);

      heat_flux += HeatTransferModels::cylindricalGapRadiationHeatFlux(
          _r_2d, r_3d, emissivity_2d, emissivity_3d, T_2d, T_3d);
    }

    heat_flux_3d[qp_3d] = heat_flux;
    heat_flux_2d[qp_2d] -= _JxW[qp_3d] * _coord[qp_3d] * heat_flux / area_2d[qp_2d];
  }

  // Store values in maps
  _elem_id_to_heat_flux[elem_id_3d] = heat_flux_3d;
  auto it = _elem_id_to_heat_flux.find(elem_id_2d);
  if (it == _elem_id_to_heat_flux.end())
    _elem_id_to_heat_flux[elem_id_2d] = heat_flux_2d;
  else
  {
    auto & heat_flux_2d_existing = _elem_id_to_heat_flux[elem_id_2d];
    for (const auto qp_2d : index_range(heat_flux_2d_existing))
      heat_flux_2d_existing[qp_2d] += heat_flux_2d[qp_2d];
  }
}

void
HSCoupler2D3DUserObject::threadJoin(const UserObject & uo)
{
  const auto & other_uo = static_cast<const HSCoupler2D3DUserObject &>(uo);
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
HSCoupler2D3DUserObject::finalize()
{
  THM::allGatherADVectorMapSum(comm(), _elem_id_to_heat_flux);
}

const std::vector<ADReal> &
HSCoupler2D3DUserObject::getHeatFlux(dof_id_type elem_id) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  auto it = _elem_id_to_heat_flux.find(elem_id);
  if (it != _elem_id_to_heat_flux.end())
    return it->second;
  else
    mooseError(name(), ": Requested heat flux for element ", elem_id, " was not computed.");
}

ADReal
HSCoupler2D3DUserObject::evaluateTemperatureFunction(const Function & fn, const ADReal & T) const
{
  ADReal f = fn.value(T);
  f.derivatives() = T.derivatives() * fn.timeDerivative(T.value());

  return f;
}
