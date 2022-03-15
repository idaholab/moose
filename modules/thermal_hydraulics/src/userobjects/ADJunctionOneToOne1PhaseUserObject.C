//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADJunctionOneToOne1PhaseUserObject.h"
#include "THMIndices3Eqn.h"
#include "ADNumericalFlux3EqnBase.h"
#include "Numerics.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "libmesh/parallel_algebra.h"

registerMooseObject("ThermalHydraulicsApp", ADJunctionOneToOne1PhaseUserObject);

const std::vector<std::pair<std::string, unsigned int>>
    ADJunctionOneToOne1PhaseUserObject::_varname_eq_index_pairs{
        std::pair<std::string, unsigned int>("rhoA", THM3Eqn::EQ_MASS),
        std::pair<std::string, unsigned int>("rhouA", THM3Eqn::EQ_MOMENTUM),
        std::pair<std::string, unsigned int>("rhoEA", THM3Eqn::EQ_ENERGY)};

Threads::spin_mutex ADJunctionOneToOne1PhaseUserObject::_spin_mutex;

InputParameters
ADJunctionOneToOne1PhaseUserObject::validParams()
{
  InputParameters params = ADFlowJunctionUserObject::validParams();

  params.addRequiredCoupledVar("A", "Cross-sectional area of connected flow channels");
  params.addRequiredCoupledVar("rhoA", "rho*A of the connected flow channels");
  params.addRequiredCoupledVar("rhouA", "rhou*A of the connected flow channels");
  params.addRequiredCoupledVar("rhoEA", "rhoE*A of the connected flow channels");

  params.addRequiredParam<UserObjectName>("numerical_flux", "Numerical flux user object name");

  params.addRequiredParam<std::string>("junction_name", "Name of the junction component");

  params.addClassDescription(
      "Computes flux between two subdomains for 1-phase one-to-one junction");

  return params;
}

ADJunctionOneToOne1PhaseUserObject::ADJunctionOneToOne1PhaseUserObject(
    const InputParameters & params)
  : ADFlowJunctionUserObject(params),

    _A(adCoupledValue("A")),
    _rhoA(adCoupledValue("rhoA")),
    _rhouA(adCoupledValue("rhouA")),
    _rhoEA(adCoupledValue("rhoEA")),

    _numerical_flux(getUserObject<ADNumericalFlux3EqnBase>("numerical_flux")),

    _junction_name(getParam<std::string>("junction_name")),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _solutions(_n_connections),
    _fluxes(_n_connections),
    _dof_indices(_n_connections, std::vector<dof_id_type>(THM3Eqn::N_EQ, 0)),

    _elem_ids(_n_connections),
    _local_side_ids(_n_connections),

    _areas(_n_connections),
    _directions(_n_connections)
{
}

void
ADJunctionOneToOne1PhaseUserObject::initialize()
{
  _connection_indices.clear();

  std::vector<ADReal> zero(THM3Eqn::N_CONS_VAR, ADReal(0.));
  for (auto & s : _solutions)
    s = zero;
}

void
ADJunctionOneToOne1PhaseUserObject::execute()
{
  // Get connection index
  const unsigned int c = getBoundaryIDIndex();
  _connection_indices.push_back(c);

  // Store DoF indices
  for (const auto & varname_eq_index_pair : _varname_eq_index_pairs)
  {
    MooseVariable * var = getVar(varname_eq_index_pair.first, 0);
    auto && dofs = var->dofIndices();
    mooseAssert(dofs.size() == 1, "There should be only one DoF index.");
    _dof_indices[c][varname_eq_index_pair.second] = dofs[0];
  }

  // Store solution vector for connection
  std::vector<ADReal> U(THM3Eqn::N_CONS_VAR, ADReal(0.));
  U[THM3Eqn::CONS_VAR_RHOA] = _rhoA[0];
  U[THM3Eqn::CONS_VAR_RHOUA] = _rhouA[0];
  U[THM3Eqn::CONS_VAR_RHOEA] = _rhoEA[0];
  U[THM3Eqn::CONS_VAR_AREA] = _A[0];
  _solutions[c] = U;

  // Store element ID and local side ID for connection
  _elem_ids[c] = _current_elem->id();
  _local_side_ids[c] = _current_side;

  // Store direction and area of channel at junction (used for error-checking)
  _areas[c] = _A[0];
  _directions[c] = _dir[0];
}

void
ADJunctionOneToOne1PhaseUserObject::threadJoin(const UserObject & uo)
{
  const ADJunctionOneToOne1PhaseUserObject & junction_uo =
      dynamic_cast<const ADJunctionOneToOne1PhaseUserObject &>(uo);

  // Store the data computed/retrieved in the other threads
  for (unsigned int i = 0; i < junction_uo._connection_indices.size(); i++)
  {
    const unsigned int c = junction_uo._connection_indices[i];

    _dof_indices[c] = junction_uo._dof_indices[c];
    _solutions[c] = junction_uo._solutions[c];
    _elem_ids[c] = junction_uo._elem_ids[c];
    _local_side_ids[c] = junction_uo._local_side_ids[c];
    _areas[c] = junction_uo._areas[c];
    _directions[c] = junction_uo._directions[c];
  }
}

void
ADJunctionOneToOne1PhaseUserObject::finalize()
{
  // Check direction compatibility
  if (!THM::areParallelVectors(_directions[0], _directions[1]))
    mooseError(_junction_name, ": The connected channels must be parallel at the junction.");

  for (unsigned int i = 0; i < _n_connections; i++)
  {
    processor_id_type owner_proc = _processor_ids[i];
    comm().broadcast(_elem_ids[i], owner_proc, true);
    comm().broadcast(_local_side_ids[i], owner_proc, true);
    comm().broadcast(_solutions[i], owner_proc, true);
    comm().broadcast(_directions[i], owner_proc, true);
  }

  // Apply transformation to first connection's reference frame
  const Real n_direction1 = _directions[0] * _directions[1];
  _solutions[1][THM3Eqn::CONS_VAR_RHOUA] *= n_direction1;

  _fluxes[0] = _numerical_flux.getFlux(
      _local_side_ids[0], _elem_ids[0], true, _solutions[0], _solutions[1], _normal[0]);

  _fluxes[1] = _numerical_flux.getFlux(
      _local_side_ids[0], _elem_ids[0], false, _solutions[0], _solutions[1], _normal[0]);
  _fluxes[1][THM3Eqn::CONS_VAR_RHOA] *= n_direction1;
  _fluxes[1][THM3Eqn::CONS_VAR_RHOEA] *= n_direction1;
}

const std::vector<ADReal> &
ADJunctionOneToOne1PhaseUserObject::getFlux(const unsigned int & connection_index) const
{
  Threads::spin_mutex::scoped_lock lock(_spin_mutex);

  return _fluxes[connection_index];
}
