//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADGateValve1PhaseUserObject.h"
#include "THMIndices3Eqn.h"
#include "ADNumericalFlux3EqnBase.h"
#include "Function.h"
#include "Numerics.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "libmesh/parallel_algebra.h"

registerMooseObject("ThermalHydraulicsApp", ADGateValve1PhaseUserObject);

const std::vector<std::pair<std::string, unsigned int>>
    ADGateValve1PhaseUserObject::_varname_eq_index_pairs{
        std::pair<std::string, unsigned int>("rhoA", THM3Eqn::EQ_MASS),
        std::pair<std::string, unsigned int>("rhouA", THM3Eqn::EQ_MOMENTUM),
        std::pair<std::string, unsigned int>("rhoEA", THM3Eqn::EQ_ENERGY)};

InputParameters
ADGateValve1PhaseUserObject::validParams()
{
  InputParameters params = ADFlowJunctionUserObject::validParams();

  params.addRequiredParam<Real>("open_area_fraction",
                                "Fraction of possible flow area that is open");
  params.addParam<Real>("open_area_fraction_min", 1e-10, "Minimum open area fraction");

  params.addRequiredCoupledVar("A", "Cross-sectional area of connected flow channels");
  params.addRequiredCoupledVar("rhoA", "rho*A of the connected flow channels");
  params.addRequiredCoupledVar("rhouA", "rhou*A of the connected flow channels");
  params.addRequiredCoupledVar("rhoEA", "rhoE*A of the connected flow channels");

  params.addRequiredParam<UserObjectName>("numerical_flux", "Numerical flux user object name");

  params.addRequiredParam<std::string>("component_name", "Name of the associated component");

  params.addClassDescription("Gate valve user object for 1-phase flow");

  params.declareControllable("open_area_fraction");

  return params;
}

ADGateValve1PhaseUserObject::ADGateValve1PhaseUserObject(const InputParameters & params)
  : ADFlowJunctionUserObject(params),

    _f_open(getParam<Real>("open_area_fraction")),
    _f_open_min(getParam<Real>("open_area_fraction_min")),

    _A(adCoupledValue("A")),
    _rhoA(adCoupledValue("rhoA")),
    _rhouA(adCoupledValue("rhouA")),
    _rhoEA(adCoupledValue("rhoEA")),

    _rhoA_jvar(coupled("rhoA")),
    _rhouA_jvar(coupled("rhouA")),
    _rhoEA_jvar(coupled("rhoEA")),

    _p(getADMaterialProperty<Real>("p")),

    _numerical_flux(getUserObject<ADNumericalFlux3EqnBase>("numerical_flux")),

    _component_name(getParam<std::string>("component_name")),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _solutions(_n_connections),
    _fluxes(_n_connections, std::vector<ADReal>(THM3Eqn::N_EQ, 0)),
    _dof_indices(_n_connections, std::vector<dof_id_type>(THM3Eqn::N_EQ, 0)),

    _stored_p(_n_connections),

    _elem_ids(_n_connections),
    _local_side_ids(_n_connections),

    _areas(_n_connections),
    _directions(_n_connections)
{
}

void
ADGateValve1PhaseUserObject::initialize()
{
  _connection_indices.clear();

  std::vector<ADReal> zero(THM3Eqn::N_CONS_VAR, ADReal(0.));
  for (auto & s : _solutions)
    s = zero;
  for (unsigned int i = 0; i < THM3Eqn::N_EQ; i++)
  {
    _fluxes[0][i] = 0;
    _fluxes[1][i] = 0;
  }
}

void
ADGateValve1PhaseUserObject::execute()
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

  _stored_p[c] = _p[0];
}

void
ADGateValve1PhaseUserObject::threadJoin(const UserObject & uo)
{
  const ADGateValve1PhaseUserObject & junction_uo =
      dynamic_cast<const ADGateValve1PhaseUserObject &>(uo);

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
    _stored_p[c] = junction_uo._stored_p[c];
  }
}

void
ADGateValve1PhaseUserObject::finalize()
{
  // Check direction compatibility
  if (!THM::areParallelVectors(_directions[0], _directions[1]))
    mooseError(_component_name, ": The connected channels must be parallel at the junction.");

  for (unsigned int i = 0; i < _n_connections; i++)
  {
    processor_id_type owner_proc = _processor_ids[i];
    comm().broadcast(_elem_ids[i], owner_proc, true);
    comm().broadcast(_local_side_ids[i], owner_proc, true);
    comm().broadcast(_solutions[i], owner_proc, true);
    comm().broadcast(_directions[i], owner_proc, true);
    comm().broadcast(_areas[i], owner_proc, true);
    comm().broadcast(_stored_p[i], owner_proc, true);
  }

  const Real & n1_dot_d1 = _normal[0];
  const Real d1_dot_d2 = _directions[0] * _directions[1];

  const ADReal & A1 = _areas[0];
  const ADReal & A2 = _areas[1];
  const ADReal A = std::min(A1, A2);
  const ADReal A_flow = _f_open * A;

  if (_f_open > _f_open_min)
  {
    // compute flow contribution
    std::vector<ADReal> U_flow1 = _solutions[0];
    std::vector<ADReal> U_flow2 = _solutions[1];
    U_flow1[THM3Eqn::CONS_VAR_AREA] = A_flow;
    U_flow2[THM3Eqn::CONS_VAR_AREA] = A_flow;
    for (unsigned int i = 0; i < THM3Eqn::N_EQ; i++)
    {
      U_flow1[i] *= A_flow / A1;
      U_flow2[i] *= A_flow / A2;
    }
    U_flow2[THM3Eqn::CONS_VAR_RHOUA] *= d1_dot_d2;

    _fluxes[0] = _numerical_flux.getFlux(
        _local_side_ids[0], _elem_ids[0], true, U_flow1, U_flow2, n1_dot_d1);

    _fluxes[1] = _numerical_flux.getFlux(
        _local_side_ids[0], _elem_ids[0], false, U_flow1, U_flow2, n1_dot_d1);
    _fluxes[1][THM3Eqn::CONS_VAR_RHOA] *= d1_dot_d2;
    _fluxes[1][THM3Eqn::CONS_VAR_RHOEA] *= d1_dot_d2;
  }

  // compute wall contribution
  for (unsigned int c = 0; c < _n_connections; c++)
  {
    const ADReal A_wall = _areas[c] - A_flow;
    _fluxes[c][THM3Eqn::CONS_VAR_RHOUA] += _stored_p[c] * A_wall;
  }
}

const std::vector<ADReal> &
ADGateValve1PhaseUserObject::getFlux(const unsigned int & connection_index) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  return _fluxes[connection_index];
}
