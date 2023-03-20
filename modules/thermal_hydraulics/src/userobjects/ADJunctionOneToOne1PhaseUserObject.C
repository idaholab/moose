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
#include "FlowModel1PhaseUtils.h"
#include "SinglePhaseFluidProperties.h"
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
  params += SlopeReconstruction1DInterface<true>::validParams();

  params.addRequiredCoupledVar(
      "A_elem", "Piecewise constant cross-sectional area of connected flow channels");
  params.addRequiredCoupledVar("A_linear",
                               "Piecewise linear cross-sectional area of connected flow channels");
  params.addRequiredCoupledVar("rhoA", "rho*A of the connected flow channels");
  params.addRequiredCoupledVar("rhouA", "rhou*A of the connected flow channels");
  params.addRequiredCoupledVar("rhoEA", "rhoE*A of the connected flow channels");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  params.addRequiredParam<UserObjectName>("numerical_flux", "Numerical flux user object name");

  params.addRequiredParam<std::string>("junction_name", "Name of the junction component");

  params.addClassDescription(
      "Computes flux between two subdomains for 1-phase one-to-one junction");

  return params;
}

ADJunctionOneToOne1PhaseUserObject::ADJunctionOneToOne1PhaseUserObject(
    const InputParameters & params)
  : ADFlowJunctionUserObject(params),
    SlopeReconstruction1DInterface<true>(this),

    _A_linear(adCoupledValue("A_linear")),

    _A_var(getVar("A_elem", 0)),
    _rhoA_var(getVar("rhoA", 0)),
    _rhouA_var(getVar("rhouA", 0)),
    _rhoEA_var(getVar("rhoEA", 0)),

    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties")),

    _numerical_flux(getUserObject<ADNumericalFlux3EqnBase>("numerical_flux")),

    _junction_name(getParam<std::string>("junction_name")),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _primitive_solutions(_n_connections),
    _neighbor_primitive_solutions(_n_connections),

    _fluxes(_n_connections),
    _dof_indices(_n_connections, std::vector<dof_id_type>(THM3Eqn::N_EQ, 0)),

    _elem_ids(_n_connections),
    _local_side_ids(_n_connections),

    _areas_linear(_n_connections),
    _directions(_n_connections),
    _positions(_n_connections),
    _neighbor_positions(_n_connections),
    _delta_x(_n_connections),
    _has_neighbor(_n_connections)
{
  _U_vars.resize(THM3Eqn::N_CONS_VAR);
  _U_vars[THM3Eqn::CONS_VAR_RHOA] = _rhoA_var;
  _U_vars[THM3Eqn::CONS_VAR_RHOUA] = _rhouA_var;
  _U_vars[THM3Eqn::CONS_VAR_RHOEA] = _rhoEA_var;
  _U_vars[THM3Eqn::CONS_VAR_AREA] = _A_var;
}

void
ADJunctionOneToOne1PhaseUserObject::initialize()
{
  _connection_indices.clear();

  // Broadcasts require vectors on all processes to have the same size
  std::vector<ADReal> zero(THM3Eqn::N_PRIM_VAR, ADReal(0.));
  for (unsigned int c = 0; c < _n_connections; c++)
  {
    _primitive_solutions[c] = zero;
    _neighbor_primitive_solutions[c] = zero;
  }
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

  // Store primitive solution vectors for connection
  const auto U_avg =
      FlowModel1PhaseUtils::getElementalSolutionVector<true>(_current_elem, _U_vars, _is_implicit);
  _primitive_solutions[c] = FlowModel1PhaseUtils::computePrimitiveSolutionVector<true>(U_avg, _fp);

  // Get the neighbor solution and position. There should be one neighbor, unless
  // there is only one element in the subdomain. Because broadcasting requires
  // data sizes to be allocated on all processes, we cannot work with a vector
  // of a size not known upfront, so we work with a single neighbor and have
  // a flag that states whether there is one or zero neighbors.
  std::vector<std::vector<GenericReal<true>>> W_neighbor;
  std::vector<Point> x_neighbor;
  getNeighborPrimitiveVariables(_current_elem, W_neighbor, x_neighbor);
  if (W_neighbor.size() == 1)
  {
    _neighbor_primitive_solutions[c] = W_neighbor[0];
    _neighbor_positions[c] = x_neighbor[0];
    _has_neighbor[c] = true;
  }
  else
    _has_neighbor[c] = false;

  // Store element ID and local side ID for connection
  _elem_ids[c] = _current_elem->id();
  _local_side_ids[c] = _current_side;

  // Store direction, area, and position of channel at junction
  _areas_linear[c] = _A_linear[0];
  _directions[c] = _dir[0];
  _positions[c] = _q_point[0];
  _delta_x[c] = (_positions[c] - _current_elem->vertex_average()) * _directions[c];
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
    _primitive_solutions[c] = junction_uo._primitive_solutions[c];
    _neighbor_primitive_solutions[c] = junction_uo._neighbor_primitive_solutions[c];
    _elem_ids[c] = junction_uo._elem_ids[c];
    _local_side_ids[c] = junction_uo._local_side_ids[c];
    _areas_linear[c] = junction_uo._areas_linear[c];
    _directions[c] = junction_uo._directions[c];
    _positions[c] = junction_uo._positions[c];
    _neighbor_positions[c] = junction_uo._neighbor_positions[c];
    _delta_x[c] = junction_uo._delta_x[c];
    _has_neighbor[c] = junction_uo._has_neighbor[c];
  }
}

void
ADJunctionOneToOne1PhaseUserObject::finalize()
{
  for (unsigned int i = 0; i < _n_connections; i++)
  {
    processor_id_type owner_proc = _processor_ids[i];
    comm().broadcast(_primitive_solutions[i], owner_proc, true);
    comm().broadcast(_neighbor_primitive_solutions[i], owner_proc, true);
    comm().broadcast(_elem_ids[i], owner_proc, true);
    comm().broadcast(_local_side_ids[i], owner_proc, true);
    comm().broadcast(_areas_linear[i], owner_proc, true);
    comm().broadcast(_directions[i], owner_proc, true);
    comm().broadcast(_positions[i], owner_proc, true);
    comm().broadcast(_neighbor_positions[i], owner_proc, true);
    comm().broadcast(_delta_x[i], owner_proc, true);

    // Vectors of bools are special, so we must broadcast a single bool instead
    bool has_neighbor = _has_neighbor[i];
    comm().broadcast(has_neighbor, owner_proc, true);
    _has_neighbor[i] = has_neighbor;
  }

  const auto & W0_avg = _primitive_solutions[0];
  const auto & W1_avg = _primitive_solutions[1];

  // Multiplier for possibly different coordinate systems
  const Real dir_mult = -_normal[0] * _normal[1];
  auto W0_ref = W0_avg;
  auto W1_ref = W1_avg;
  W0_ref[THM3Eqn::PRIM_VAR_VELOCITY] *= dir_mult;
  W1_ref[THM3Eqn::PRIM_VAR_VELOCITY] *= dir_mult;

  std::vector<std::vector<GenericReal<true>>> W_neighbor0;
  std::vector<Point> x_neighbor0;
  if (_has_neighbor[0])
  {
    W_neighbor0.push_back(_neighbor_primitive_solutions[0]);
    x_neighbor0.push_back(_neighbor_positions[0]);
  }

  std::vector<std::vector<GenericReal<true>>> W_neighbor1;
  std::vector<Point> x_neighbor1;
  if (_has_neighbor[1])
  {
    W_neighbor1.push_back(_neighbor_primitive_solutions[1]);
    x_neighbor1.push_back(_neighbor_positions[1]);
  }

  const auto slopes0 = getBoundaryElementSlopes(
      W0_avg, _positions[0], _directions[0], W_neighbor0, x_neighbor0, W1_ref);
  const auto slopes1 = getBoundaryElementSlopes(
      W1_avg, _positions[1], _directions[1], W_neighbor1, x_neighbor1, W0_ref);

  auto W0 = W0_avg;
  auto W1 = W1_avg;
  for (unsigned int m = 0; m < THM3Eqn::N_PRIM_VAR; m++)
  {
    W0[m] = W0_avg[m] + slopes0[m] * _delta_x[0];
    W1[m] = W1_avg[m] + slopes1[m] * _delta_x[1];
  }

  auto U0 =
      FlowModel1PhaseUtils::computeConservativeSolutionVector<true>(W0, _areas_linear[0], _fp);
  auto U1 =
      FlowModel1PhaseUtils::computeConservativeSolutionVector<true>(W1, _areas_linear[1], _fp);
  U1[THM3Eqn::CONS_VAR_RHOUA] *= dir_mult;

  _fluxes[0] = _numerical_flux.getFlux(_local_side_ids[0], _elem_ids[0], true, U0, U1, _normal[0]);

  _fluxes[1] = _numerical_flux.getFlux(_local_side_ids[0], _elem_ids[0], false, U0, U1, _normal[0]);
  _fluxes[1][THM3Eqn::CONS_VAR_RHOA] *= dir_mult;
  _fluxes[1][THM3Eqn::CONS_VAR_RHOEA] *= dir_mult;
}

const std::vector<ADReal> &
ADJunctionOneToOne1PhaseUserObject::getFlux(const unsigned int & connection_index) const
{
  Threads::spin_mutex::scoped_lock lock(_spin_mutex);

  return _fluxes[connection_index];
}

std::vector<ADReal>
ADJunctionOneToOne1PhaseUserObject::computeElementPrimitiveVariables(const Elem * elem) const
{
  const auto U =
      FlowModel1PhaseUtils::getElementalSolutionVector<true>(elem, _U_vars, _is_implicit);
  return FlowModel1PhaseUtils::computePrimitiveSolutionVector<true>(U, _fp);
}
