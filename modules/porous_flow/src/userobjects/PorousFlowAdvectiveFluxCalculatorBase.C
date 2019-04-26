//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAdvectiveFluxCalculatorBase.h"
#include "Assembly.h"
#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<PorousFlowAdvectiveFluxCalculatorBase>()
{
  InputParameters params = validParams<AdvectiveFluxCalculatorBase>();
  params.addClassDescription(
      "Base class to compute the advective flux of fluid in PorousFlow situations.  The velocity "
      "is U * (-permeability * (grad(P) - density * gravity)), while derived classes define U.  "
      "The Kuzmin-Turek FEM-TVD multidimensional stabilization scheme is used");
  params.addRequiredParam<RealVectorValue>("gravity",
                                           "Gravitational acceleration vector downwards (m/s^2)");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addParam<unsigned int>(
      "phase", 0, "The index corresponding to the phase for this UserObject");
  MooseEnum families("LAGRANGE MONOMIAL HERMITE SCALAR HIERARCHIC CLOUGH XYZ SZABAB BERNSTEIN");
  params.addParam<MooseEnum>(
      "fe_family",
      families,
      "FE Family to use (eg Lagrange).  You only need to specify this is your porous_flow_vars in "
      "your PorousFlowDictator have different families or orders");
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH");
  params.addParam<MooseEnum>(
      "fe_order",
      orders,
      "FE Order to use (eg First).  You only need to specify this is your porous_flow_vars in your "
      "PorousFlowDictator have different families or orders");
  return params;
}

PorousFlowAdvectiveFluxCalculatorBase::PorousFlowAdvectiveFluxCalculatorBase(
    const InputParameters & parameters)
  : AdvectiveFluxCalculatorBase(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_vars(_dictator.numVariables()),
    _gravity(getParam<RealVectorValue>("gravity")),
    _phase(getParam<unsigned int>("phase")),
    _permeability(getMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _dpermeability_dvar(
        getMaterialProperty<std::vector<RealTensorValue>>("dPorousFlow_permeability_qp_dvar")),
    _dpermeability_dgradvar(getMaterialProperty<std::vector<std::vector<RealTensorValue>>>(
        "dPorousFlow_permeability_qp_dgradvar")),
    _fluid_density_qp(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _dfluid_density_qp_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_density_qp_dvar")),
    _grad_p(getMaterialProperty<std::vector<RealGradient>>("PorousFlow_grad_porepressure_qp")),
    _dgrad_p_dgrad_var(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_grad_porepressure_qp_dgradvar")),
    _dgrad_p_dvar(getMaterialProperty<std::vector<std::vector<RealGradient>>>(
        "dPorousFlow_grad_porepressure_qp_dvar")),
    _fe_type(isParamValid("fe_family") && isParamValid("fe_order")
                 ? FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("fe_order")),
                          Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("fe_family")))
                 : _dictator.feType()),
    _phi(_assembly.fePhi<Real>(_fe_type)),
    _grad_phi(_assembly.feGradPhi<Real>(_fe_type)),
    _du_dvar(),
    _du_dvar_computed_by_thread(),
    _dkij_dvar(),
    _dflux_out_dvars()

{
  if (_phase >= _dictator.numPhases())
    paramError("phase",
               "Phase number entered is greater than the number of phases specified in the "
               "Dictator. Remember that indexing starts at 0");

  if (isParamValid("fe_family") && !isParamValid("fe_order"))
    paramError("fe_order", "If you specify fe_family you must also specify fe_order");
  if (isParamValid("fe_order") && !isParamValid("fe_family"))
    paramError("fe_family", "If you specify fe_order you must also specify fe_family");
  if (!_dictator.consistentFEType() && !isParamValid("fe_family"))
    paramError("fe_family",
               "The PorousFlowDictator cannot determine the appropriate FE type to use because "
               "your porous_flow_vars are of different types.  You must specify the appropriate "
               "fe_family and fe_order to use.");
}

Real
PorousFlowAdvectiveFluxCalculatorBase::computeVelocity(unsigned i, unsigned j, unsigned qp) const
{
  // The following is but one choice for PorousFlow situations
  // If you change this, you will probably have to change
  // - the derivative in executeOnElement
  // - computeU
  // - computedU_dvar
  return -_grad_phi[i][qp] *
         (_permeability[qp] * (_grad_p[qp][_phase] - _fluid_density_qp[qp][_phase] * _gravity)) *
         _phi[j][qp];
}

void
PorousFlowAdvectiveFluxCalculatorBase::executeOnElement(
    dof_id_type global_i, dof_id_type global_j, unsigned local_i, unsigned local_j, unsigned qp)
{
  AdvectiveFluxCalculatorBase::executeOnElement(global_i, global_j, local_i, local_j, qp);
  const dof_id_type sequential_i = _connections.sequentialID(global_i);
  const unsigned j = _connections.indexOfGlobalConnection(global_i, global_j);

  // compute d(Kij)/d(porous_flow_variables)
  for (unsigned local_k = 0; local_k < _current_elem->n_nodes(); ++local_k)
  {
    const dof_id_type global_k = _current_elem->node_id(local_k);
    for (unsigned pvar = 0; pvar < _num_vars; ++pvar)
    {
      RealVectorValue deriv = _dpermeability_dvar[qp][pvar] * _phi[local_k][qp] *
                              (_grad_p[qp][_phase] - _fluid_density_qp[qp][_phase] * _gravity);
      for (unsigned i = 0; i < LIBMESH_DIM; ++i)
        deriv += _dpermeability_dgradvar[qp][i][pvar] * _grad_phi[local_k][qp](i) *
                 (_grad_p[qp][_phase] - _fluid_density_qp[qp][_phase] * _gravity);
      deriv += _permeability[qp] *
               (_grad_phi[local_k][qp] * _dgrad_p_dgrad_var[qp][_phase][pvar] -
                _phi[local_k][qp] * _dfluid_density_qp_dvar[qp][_phase][pvar] * _gravity);
      deriv += _permeability[qp] * (_dgrad_p_dvar[qp][_phase][pvar] * _phi[local_k][qp]);
      _dkij_dvar[sequential_i][j][global_k][pvar] +=
          _JxW[qp] * _coord[qp] * (-_grad_phi[local_i][qp] * deriv * _phi[local_j][qp]);
    }
  }
}

void
PorousFlowAdvectiveFluxCalculatorBase::timestepSetup()
{
  const bool resizing_was_needed =
      _resizing_needed; // _resizing_needed gets set to false at the end of
                        // AdvectiveFluxCalculatorBase::timestepSetup()
  AdvectiveFluxCalculatorBase::timestepSetup();

  // clear and appropriately size all the derivative info
  // d(U)/d(porous_flow_variables) and
  // d(Kij)/d(porous_flow_variables) and
  // d(flux_out)/d(porous_flow_variables)
  if (resizing_was_needed)
  {
    const std::size_t num_nodes = _connections.numNodes();
    _du_dvar.assign(num_nodes, std::vector<Real>(_num_vars, 0.0));
    _du_dvar_computed_by_thread.assign(num_nodes, false);
    _dflux_out_dvars.assign(num_nodes, std::map<dof_id_type, std::vector<Real>>());
    _dkij_dvar.resize(num_nodes);
    for (dof_id_type sequential_i = 0; sequential_i < num_nodes; ++sequential_i)
    {
      const std::vector<dof_id_type> con_i =
          _connections.globalConnectionsToSequentialID(sequential_i);
      const std::size_t num_con_i = con_i.size();
      _dkij_dvar[sequential_i].assign(num_con_i, std::map<dof_id_type, std::vector<Real>>());
      for (unsigned j = 0; j < num_con_i; ++j)
        for (const auto & global_neighbor_to_i : con_i)
          _dkij_dvar[sequential_i][j][global_neighbor_to_i] = std::vector<Real>(_num_vars, 0.0);
    }
  }
}

void
PorousFlowAdvectiveFluxCalculatorBase::initialize()
{
  AdvectiveFluxCalculatorBase::initialize();
  const std::size_t num_nodes = _connections.numNodes();
  _du_dvar_computed_by_thread.assign(num_nodes, false);
  for (dof_id_type sequential_i = 0; sequential_i < num_nodes; ++sequential_i)
  {
    const std::vector<dof_id_type> con_i =
        _connections.globalConnectionsToSequentialID(sequential_i);
    const std::size_t num_con_i = con_i.size();
    for (unsigned j = 0; j < num_con_i; ++j)
      for (const auto & global_neighbor_to_i : con_i)
        _dkij_dvar[sequential_i][j][global_neighbor_to_i] = std::vector<Real>(_num_vars, 0.0);
  }
}

void
PorousFlowAdvectiveFluxCalculatorBase::execute()
{
  AdvectiveFluxCalculatorBase::execute();

  // compute d(U)/d(porous_flow_variables) for nodes in _current_elem and for this
  // execution thread.  In threadJoin all these computations get gathered
  // using _du_dvar_computed_by_thread
  for (unsigned i = 0; i < _current_elem->n_nodes(); ++i)
  {
    const dof_id_type global_i = _current_elem->node_id(i);
    const dof_id_type sequential_i = _connections.sequentialID(global_i);
    if (_du_dvar_computed_by_thread[sequential_i])
      continue;
    for (unsigned pvar = 0; pvar < _num_vars; ++pvar)
      _du_dvar[sequential_i][pvar] = computedU_dvar(i, pvar);
    _du_dvar_computed_by_thread[sequential_i] = true;
  }
}

void
PorousFlowAdvectiveFluxCalculatorBase::threadJoin(const UserObject & uo)
{
  AdvectiveFluxCalculatorBase::threadJoin(uo);
  const PorousFlowAdvectiveFluxCalculatorBase & pfafc =
      static_cast<const PorousFlowAdvectiveFluxCalculatorBase &>(uo);
  // add the values of _dkij_dvar computed by different threads
  const std::size_t num_nodes = _connections.numNodes();
  for (dof_id_type sequential_i = 0; sequential_i < num_nodes; ++sequential_i)
  {
    const std::vector<dof_id_type> con_i =
        _connections.globalConnectionsToSequentialID(sequential_i);
    const std::size_t num_con_i = con_i.size();
    for (unsigned j = 0; j < num_con_i; ++j)
      for (const auto & global_derivs : pfafc._dkij_dvar[sequential_i][j])
        for (unsigned pvar = 0; pvar < _num_vars; ++pvar)
          _dkij_dvar[sequential_i][j][global_derivs.first][pvar] += global_derivs.second[pvar];
  }

  // gather the values of _du_dvar computed by different threads
  for (dof_id_type sequential_i = 0; sequential_i < _number_of_nodes; ++sequential_i)
    if (!_du_dvar_computed_by_thread[sequential_i] &&
        pfafc._du_dvar_computed_by_thread[sequential_i])
      _du_dvar[sequential_i] = pfafc._du_dvar[sequential_i];
}


const std::map<dof_id_type, std::vector<Real>> &
PorousFlowAdvectiveFluxCalculatorBase::getdK_dvar(dof_id_type node_i, dof_id_type node_j) const
{
  const dof_id_type sequential_i = _connections.sequentialID(node_i);
  const unsigned j = _connections.indexOfGlobalConnection(node_i, node_j);
  return _dkij_dvar[sequential_i][j];
}

const std::map<dof_id_type, std::vector<Real>> &
PorousFlowAdvectiveFluxCalculatorBase::getdFluxOut_dvars(unsigned node_id) const
{
  return _dflux_out_dvars[_connections.sequentialID(node_id)];
}

void
PorousFlowAdvectiveFluxCalculatorBase::finalize()
{
  AdvectiveFluxCalculatorBase::finalize();

  // compute d(flux_out)/d(porous flow variable)
  /// _dflux_out_dvars[sequential_i][global_j][pvar] = d(flux_out[global version of sequential_i])/d(porous_flow_variable pvar at global node j)
  for (const auto & node_i : _connections.globalIDs())
  {
    const dof_id_type sequential_i = _connections.sequentialID(node_i);
    _dflux_out_dvars[sequential_i].clear();

    const std::map<dof_id_type, Real> dflux_out_du =
        AdvectiveFluxCalculatorBase::getdFluxOutdu(node_i);
    for (const auto & node_du : dflux_out_du)
    {
      const dof_id_type j = node_du.first;
      const Real dflux_out_du_j = node_du.second;
      _dflux_out_dvars[sequential_i][j] = _du_dvar[_connections.sequentialID(j)];
      for (unsigned pvar = 0; pvar < _num_vars; ++pvar)
        _dflux_out_dvars[sequential_i][j][pvar] *= dflux_out_du_j;
    }

    // _dflux_out_dvars is now sized correctly, because getdFluxOutdu(i) contains all nodes
    // connected to i and all nodes connected to nodes connected to i.  The
    // getdFluxOutdKij contains no extra nodes, so just += the dflux/dK terms
    const std::vector<std::vector<Real>> dflux_out_dKjk =
        AdvectiveFluxCalculatorBase::getdFluxOutdKjk(node_i);
    const std::vector<dof_id_type> con_i = _connections.globalConnectionsToGlobalID(node_i);
    for (std::size_t index_j = 0; index_j < con_i.size(); ++index_j)
    {
      const dof_id_type node_j = con_i[index_j];
      const std::vector<dof_id_type> con_j = _connections.globalConnectionsToGlobalID(node_j);
      for (std::size_t index_k = 0; index_k < con_j.size(); ++index_k)
      {
        const dof_id_type node_k = con_j[index_k];
        const Real dflux_out_dK_jk = dflux_out_dKjk[index_j][index_k];
        const std::map<dof_id_type, std::vector<Real>> dkj_dvarl = getdK_dvar(node_j, node_k);
        for (const auto & nodel_deriv : dkj_dvarl)
        {
          const dof_id_type l = nodel_deriv.first;
          for (unsigned pvar = 0; pvar < _num_vars; ++pvar)
            _dflux_out_dvars[sequential_i][l][pvar] += dflux_out_dK_jk * nodel_deriv.second[pvar];
        }
      }
    }
  }
}
