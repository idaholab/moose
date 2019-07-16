#include "GateValve1PhaseUserObject.h"
#include "THMIndices3Eqn.h"
#include "NumericalFlux3EqnBase.h"
#include "Function.h"
#include "Numerics.h"

registerMooseObject("THMApp", GateValve1PhaseUserObject);

const std::vector<std::pair<std::string, unsigned int>>
    GateValve1PhaseUserObject::_varname_eq_index_pairs{
        std::pair<std::string, unsigned int>("rhoA", THM3Eqn::EQ_MASS),
        std::pair<std::string, unsigned int>("rhouA", THM3Eqn::EQ_MOMENTUM),
        std::pair<std::string, unsigned int>("rhoEA", THM3Eqn::EQ_ENERGY)};

template <>
InputParameters
validParams<GateValve1PhaseUserObject>()
{
  InputParameters params = validParams<FlowJunctionUserObject>();

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

GateValve1PhaseUserObject::GateValve1PhaseUserObject(const InputParameters & params)
  : DerivativeMaterialInterfaceTHM<FlowJunctionUserObject>(params),

    _f_open(getParam<Real>("open_area_fraction")),
    _f_open_min(getParam<Real>("open_area_fraction_min")),

    _A(coupledValue("A")),
    _rhoA(coupledValue("rhoA")),
    _rhouA(coupledValue("rhouA")),
    _rhoEA(coupledValue("rhoEA")),

    _rhoA_jvar(coupled("rhoA")),
    _rhouA_jvar(coupled("rhouA")),
    _rhoEA_jvar(coupled("rhoEA")),

    _p(getMaterialProperty<Real>("p")),
    _dp_drhoA(getMaterialPropertyDerivativeTHM<Real>("p", "rhoA")),
    _dp_drhouA(getMaterialPropertyDerivativeTHM<Real>("p", "rhouA")),
    _dp_drhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "rhoEA")),

    _numerical_flux(getUserObject<NumericalFlux3EqnBase>("numerical_flux")),

    _component_name(getParam<std::string>("component_name")),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _solutions(_n_connections),
    _fluxes(_n_connections, std::vector<Real>(THM3Eqn::N_EQ, 0)),
    _flux_jacobians(
        _n_connections,
        std::vector<DenseMatrix<Real>>(2, DenseMatrix<Real>(THM3Eqn::N_EQ, THM3Eqn::N_EQ))),
    _dof_indices(_n_connections, std::vector<dof_id_type>(THM3Eqn::N_EQ, 0)),

    _stored_p(_n_connections),
    _stored_dp_drhoA(_n_connections),
    _stored_dp_drhouA(_n_connections),
    _stored_dp_drhoEA(_n_connections),

    _elem_ids(_n_connections),
    _local_side_ids(_n_connections),

    _areas(_n_connections),
    _directions(_n_connections)
{
}

void
GateValve1PhaseUserObject::initialize()
{
  _connection_indices.clear();

  for (unsigned int i = 0; i < THM3Eqn::N_EQ; i++)
  {
    _fluxes[0][i] = 0;
    _fluxes[1][i] = 0;
  }
  _flux_jacobians[0][0].zero();
  _flux_jacobians[0][1].zero();
  _flux_jacobians[1][0].zero();
  _flux_jacobians[1][1].zero();
}

void
GateValve1PhaseUserObject::execute()
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

  _stored_p[c] = _p[0];
  _stored_dp_drhoA[c] = _dp_drhoA[0];
  _stored_dp_drhouA[c] = _dp_drhouA[0];
  _stored_dp_drhoEA[c] = _dp_drhoEA[0];
}

void
GateValve1PhaseUserObject::threadJoin(const UserObject & uo)
{
  const GateValve1PhaseUserObject & junction_uo =
      dynamic_cast<const GateValve1PhaseUserObject &>(uo);

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
    _stored_dp_drhoA[c] = junction_uo._stored_dp_drhoA[c];
    _stored_dp_drhouA[c] = junction_uo._stored_dp_drhouA[c];
    _stored_dp_drhoEA[c] = junction_uo._stored_dp_drhoEA[c];
  }
}

void
GateValve1PhaseUserObject::finalize()
{
  // Check direction compatibility
  if (!THM::areParallelVectors(_directions[0], _directions[1]))
    mooseError(_component_name, ": The connected channels must be parallel at the junction.");

  const Real & n1_dot_d1 = _normal[0];
  const Real d1_dot_d2 = _directions[0] * _directions[1];

  const Real & A1 = _areas[0];
  const Real & A2 = _areas[1];
  const Real A = std::min(A1, A2);
  const Real A_flow = _f_open * A;

  if (_f_open > _f_open_min)
  {
    // compute flow contribution
    std::vector<Real> U_flow1 = _solutions[0];
    std::vector<Real> U_flow2 = _solutions[1];
    U_flow1[THM3Eqn::CONS_VAR_AREA] = A_flow;
    U_flow2[THM3Eqn::CONS_VAR_AREA] = A_flow;
    for (unsigned int i = 0; i < THM3Eqn::N_EQ; i++)
    {
      U_flow1[i] *= A_flow / A1;
      U_flow2[i] *= A_flow / A2;
    }
    U_flow2[THM3Eqn::CONS_VAR_RHOUA] *= d1_dot_d2;

    _fluxes[0] =
        _numerical_flux.getFlux(_local_side_ids[0], _elem_ids[0], U_flow1, U_flow2, n1_dot_d1);

    _fluxes[1] = _fluxes[0];
    _fluxes[1][THM3Eqn::CONS_VAR_RHOA] *= d1_dot_d2;
    _fluxes[1][THM3Eqn::CONS_VAR_RHOEA] *= d1_dot_d2;

    _flux_jacobians[0][0] = _numerical_flux.getJacobian(
        true, _local_side_ids[0], _elem_ids[0], U_flow1, U_flow2, n1_dot_d1);
    _flux_jacobians[0][1] = _numerical_flux.getJacobian(
        false, _local_side_ids[0], _elem_ids[0], U_flow1, U_flow2, n1_dot_d1);

    _flux_jacobians[0][0].scale(A_flow / A1);
    _flux_jacobians[0][1].scale(A_flow / A2);
    for (unsigned int i = 0; i < THM3Eqn::N_EQ; i++)
      _flux_jacobians[0][1](i, THM3Eqn::CONS_VAR_RHOUA) *= d1_dot_d2;

    _flux_jacobians[1] = _flux_jacobians[0];
    for (unsigned int j = 0; j < THM3Eqn::N_EQ; j++)
    {
      _flux_jacobians[1][0](THM3Eqn::CONS_VAR_RHOA, j) *= d1_dot_d2;
      _flux_jacobians[1][0](THM3Eqn::CONS_VAR_RHOEA, j) *= d1_dot_d2;
      _flux_jacobians[1][1](THM3Eqn::CONS_VAR_RHOA, j) *= d1_dot_d2;
      _flux_jacobians[1][1](THM3Eqn::CONS_VAR_RHOEA, j) *= d1_dot_d2;
    }
  }

  // compute wall contribution
  for (unsigned int c = 0; c < _n_connections; c++)
  {
    const Real A_wall = _areas[c] - A_flow;
    _fluxes[c][THM3Eqn::CONS_VAR_RHOUA] += _stored_p[c] * A_wall;
    _flux_jacobians[c][c](THM3Eqn::CONS_VAR_RHOUA, THM3Eqn::CONS_VAR_RHOA) +=
        _stored_dp_drhoA[c] * A_wall;
    _flux_jacobians[c][c](THM3Eqn::CONS_VAR_RHOUA, THM3Eqn::CONS_VAR_RHOUA) +=
        _stored_dp_drhouA[c] * A_wall;
    _flux_jacobians[c][c](THM3Eqn::CONS_VAR_RHOUA, THM3Eqn::CONS_VAR_RHOEA) +=
        _stored_dp_drhoEA[c] * A_wall;
  }
}

const std::vector<Real> &
GateValve1PhaseUserObject::getFlux(const unsigned int & connection_index) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  return _fluxes[connection_index];
}

void
GateValve1PhaseUserObject::getJacobianEntries(const unsigned int & connection_index,
                                              const unsigned int & equation_i,
                                              const unsigned int & equation_j,
                                              DenseMatrix<Real> & jacobian_block,
                                              std::vector<dof_id_type> & dofs_j) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  jacobian_block.resize(1, _n_connections);
  dofs_j.resize(_n_connections);
  for (unsigned int c = 0; c < _n_connections; c++)
  {
    jacobian_block(0, c) = _flux_jacobians[connection_index][c](equation_i, equation_j);
    dofs_j[c] = _dof_indices[c][equation_j];
  }
}
