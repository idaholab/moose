#include "JunctionOneToOne1PhaseUserObject.h"
#include "THMIndices3Eqn.h"
#include "NumericalFlux3EqnBase.h"
#include "Numerics.h"

registerMooseObject("THMApp", JunctionOneToOne1PhaseUserObject);

const std::vector<std::pair<std::string, unsigned int>>
    JunctionOneToOne1PhaseUserObject::_varname_eq_index_pairs{
        std::pair<std::string, unsigned int>("rhoA", THM3Eqn::EQ_MASS),
        std::pair<std::string, unsigned int>("rhouA", THM3Eqn::EQ_MOMENTUM),
        std::pair<std::string, unsigned int>("rhoEA", THM3Eqn::EQ_ENERGY)};

Threads::spin_mutex JunctionOneToOne1PhaseUserObject::_spin_mutex;

template <>
InputParameters
validParams<JunctionOneToOne1PhaseUserObject>()
{
  InputParameters params = validParams<FlowJunctionUserObject>();

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

JunctionOneToOne1PhaseUserObject::JunctionOneToOne1PhaseUserObject(const InputParameters & params)
  : FlowJunctionUserObject(params),

    _A(coupledValue("A")),
    _rhoA(coupledValue("rhoA")),
    _rhouA(coupledValue("rhouA")),
    _rhoEA(coupledValue("rhoEA")),

    _rhoA_jvar(coupled("rhoA")),
    _rhouA_jvar(coupled("rhouA")),
    _rhoEA_jvar(coupled("rhoEA")),

    _numerical_flux(getUserObject<NumericalFlux3EqnBase>("numerical_flux")),

    _junction_name(getParam<std::string>("junction_name")),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _solutions(_n_connections),
    _fluxes(_n_connections),
    _flux_jacobians(_n_connections, std::vector<DenseMatrix<Real>>(2)),
    _dof_indices(_n_connections, std::vector<dof_id_type>(THM3Eqn::N_EQ, 0)),

    _elem_ids(_n_connections),
    _local_side_ids(_n_connections),

    _areas(_n_connections),
    _directions(_n_connections)
{
}

void
JunctionOneToOne1PhaseUserObject::initialize()
{
  _connection_indices.clear();
}

void
JunctionOneToOne1PhaseUserObject::execute()
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
  std::vector<Real> U(THM3Eqn::N_CONS_VAR, 0.);
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
JunctionOneToOne1PhaseUserObject::threadJoin(const UserObject & uo)
{
  const JunctionOneToOne1PhaseUserObject & junction_uo =
      dynamic_cast<const JunctionOneToOne1PhaseUserObject &>(uo);

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
JunctionOneToOne1PhaseUserObject::finalize()
{
  // Check area and direction compatibility
  if (!MooseUtils::absoluteFuzzyEqual(_areas[0], _areas[1]))
    mooseError(_junction_name, ": The connected channels must have equal area at the junction.");
  if (!THM::areParallelVectors(_directions[0], _directions[1]))
    mooseError(_junction_name, ": The connected channels must be parallel at the junction.");

  // Apply transformation to first connection's reference frame
  const Real n_direction1 = _directions[0] * _directions[1];
  _solutions[1][THM3Eqn::CONS_VAR_RHOUA] *= n_direction1;

  _fluxes[0] = _numerical_flux.getFlux(
      _local_side_ids[0], _elem_ids[0], _solutions[0], _solutions[1], _normal[0]);

  _fluxes[1] = _fluxes[0];
  _fluxes[1][THM3Eqn::CONS_VAR_RHOA] *= n_direction1;
  _fluxes[1][THM3Eqn::CONS_VAR_RHOEA] *= n_direction1;

  _flux_jacobians[0][0] = _numerical_flux.getJacobian(
      true, _local_side_ids[0], _elem_ids[0], _solutions[0], _solutions[1], _normal[0]);
  _flux_jacobians[0][1] = _numerical_flux.getJacobian(
      false, _local_side_ids[0], _elem_ids[0], _solutions[0], _solutions[1], _normal[0]);

  // Perform chain rule for reference frame transformation
  _flux_jacobians[0][1](THM3Eqn::CONS_VAR_RHOA, THM3Eqn::CONS_VAR_RHOUA) *= n_direction1;
  _flux_jacobians[0][1](THM3Eqn::CONS_VAR_RHOUA, THM3Eqn::CONS_VAR_RHOUA) *= n_direction1;
  _flux_jacobians[0][1](THM3Eqn::CONS_VAR_RHOEA, THM3Eqn::CONS_VAR_RHOUA) *= n_direction1;

  _flux_jacobians[1] = _flux_jacobians[0];
  for (unsigned int j = 0; j < THM3Eqn::N_EQ; j++)
  {
    _flux_jacobians[1][0](THM3Eqn::CONS_VAR_RHOA, j) *= n_direction1;
    _flux_jacobians[1][0](THM3Eqn::CONS_VAR_RHOEA, j) *= n_direction1;
    _flux_jacobians[1][1](THM3Eqn::CONS_VAR_RHOA, j) *= n_direction1;
    _flux_jacobians[1][1](THM3Eqn::CONS_VAR_RHOEA, j) *= n_direction1;
  }
}

const std::vector<Real> &
JunctionOneToOne1PhaseUserObject::getFlux(const unsigned int & connection_index) const
{
  Threads::spin_mutex::scoped_lock lock(_spin_mutex);

  return _fluxes[connection_index];
}

void
JunctionOneToOne1PhaseUserObject::getJacobianEntries(const unsigned int & connection_index,
                                                     const unsigned int & equation_i,
                                                     const unsigned int & equation_j,
                                                     DenseMatrix<Real> & jacobian_block,
                                                     std::vector<dof_id_type> & dofs_j) const
{
  Threads::spin_mutex::scoped_lock lock(_spin_mutex);

  jacobian_block.resize(1, _n_connections);
  dofs_j.resize(_n_connections);
  for (unsigned int c = 0; c < _n_connections; c++)
  {
    jacobian_block(0, c) = _flux_jacobians[connection_index][c](equation_i, equation_j);
    dofs_j[c] = _dof_indices[c][equation_j];
  }
}
