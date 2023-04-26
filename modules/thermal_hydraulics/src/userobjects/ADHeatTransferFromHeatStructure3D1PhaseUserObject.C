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
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"

registerMooseObject("ThermalHydraulicsApp", ADHeatTransferFromHeatStructure3D1PhaseUserObject);

InputParameters
ADHeatTransferFromHeatStructure3D1PhaseUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredParam<MeshAlignment1D3D *>("_mesh_alignment", "Mesh alignment object");
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
    _mesh_alignment(*getParam<MeshAlignment1D3D *>("_mesh_alignment")),
    _P_hf(adCoupledValue("P_hf")),
    _Hw(getADMaterialProperty<Real>("Hw")),
    _T(getADMaterialProperty<Real>("T"))
{
  _mesh_alignment.buildCoupledElemQpIndexMap(_assembly);
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
  const auto & primary_elem_id = _current_elem->id();

  const auto & secondary_elem_ids = _mesh_alignment.getCoupledSecondaryElemIDs(primary_elem_id);
  for (const auto & secondary_elem_id : secondary_elem_ids)
  {
    const auto secondary_n_qps =
        _mesh_alignment.getSecondaryNumberOfQuadraturePoints(secondary_elem_id);

    _T_fluid[secondary_elem_id].resize(secondary_n_qps);
    _htc[secondary_elem_id].resize(secondary_n_qps);
    _heated_perimeter[secondary_elem_id].resize(secondary_n_qps);

    for (unsigned int secondary_qp = 0; secondary_qp < secondary_n_qps; secondary_qp++)
    {
      const auto primary_qp =
          _mesh_alignment.getCoupledPrimaryElemQpIndex(secondary_elem_id, secondary_qp);

      _T_fluid[secondary_elem_id][secondary_qp] = _T[primary_qp];
      _htc[secondary_elem_id][secondary_qp] = _Hw[primary_qp];
      _heated_perimeter[secondary_elem_id][secondary_qp] = _P_hf[primary_qp];
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
