//* This file is part of the MOOSE framework
//* https://www.mooseframework.org All rights reserved, see COPYRIGHT
//* for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT Licensed
//* under LGPL 2.1, please see LICENSE for details
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
    _du_dvar({}),
    _du_dvar_computed_by_thread({}),
    _dkij_dvar({})

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
  // - coputedU_dvar
  return -_grad_phi[i][qp] *
         (_permeability[qp] * (_grad_p[qp][_phase] - _fluid_density_qp[qp][_phase] * _gravity)) *
         _phi[j][qp];
}

void
PorousFlowAdvectiveFluxCalculatorBase::executeOnElement(
    dof_id_type global_i, dof_id_type global_j, unsigned local_i, unsigned local_j, unsigned qp)
{
  AdvectiveFluxCalculatorBase::executeOnElement(global_i, global_j, local_i, local_j, qp);

  // compute d(Kij)/d(porous_flow_variables)
  for (unsigned local_k = 0; local_k < _current_elem->n_nodes(); ++local_k)
  {
    const dof_id_type global_k = _current_elem->node_id(local_k);
    for (unsigned pvar = 0; pvar < _dictator.numVariables(); ++pvar)
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
      _dkij_dvar[global_i][global_j][global_k][pvar] +=
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
  // d(U)/d(porous_flow_variables) and d(Kij)/d(porous_flow_variables)
  if (resizing_was_needed)
  {
    _du_dvar.clear();
    _du_dvar_computed_by_thread.clear();
    for (const auto & nodes : _kij)
    {
      const dof_id_type node_i = nodes.first;
      _du_dvar[node_i] = std::vector<Real>(_dictator.numVariables(), 0.0);
      _du_dvar_computed_by_thread[node_i] = false;
    }

    _dkij_dvar.clear();
    for (const auto & nodes : _kij)
    {
      const dof_id_type node_i = nodes.first;
      _dkij_dvar[node_i] = {};
      for (const auto & neighbours_i : nodes.second)
      {
        const dof_id_type node_j = neighbours_i.first;
        _dkij_dvar[node_i][node_j] = {};
        for (const auto & also_neighbours_i : nodes.second)
        {
          const dof_id_type node_k = also_neighbours_i.first;
          _dkij_dvar[node_i][node_j][node_k] = std::vector<Real>(_dictator.numVariables(), 0.0);
        }
      }
    }
  }
}

void
PorousFlowAdvectiveFluxCalculatorBase::initialize()
{
  AdvectiveFluxCalculatorBase::initialize();
  for (const auto & nodes : _kij)
    _du_dvar_computed_by_thread[nodes.first] = false;
  for (auto & i_and_jmap : _dkij_dvar)
    for (auto & j_and_kmap : i_and_jmap.second)
      for (auto & k_and_derivs : j_and_kmap.second)
        k_and_derivs.second = std::vector<Real>(_dictator.numVariables(), 0.0);
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
    const dof_id_type node_i = _current_elem->node_id(i);
    if (_du_dvar_computed_by_thread[node_i])
      continue;
    for (unsigned pvar = 0; pvar < _dictator.numVariables(); ++pvar)
      _du_dvar[node_i][pvar] = computedU_dvar(i, pvar);
    _du_dvar_computed_by_thread[node_i] = true;
  }
}

void
PorousFlowAdvectiveFluxCalculatorBase::threadJoin(const UserObject & uo)
{
  AdvectiveFluxCalculatorBase::threadJoin(uo);
  const PorousFlowAdvectiveFluxCalculatorBase & pfafc =
      static_cast<const PorousFlowAdvectiveFluxCalculatorBase &>(uo);
  // add the values of _dkij_dvar computed by different threads
  for (const auto & i_and_jmap : pfafc._dkij_dvar)
  {
    const dof_id_type i = i_and_jmap.first;
    for (const auto & j_and_kmap : i_and_jmap.second)
    {
      const dof_id_type j = j_and_kmap.first;
      for (const auto & k_and_derivs : j_and_kmap.second)
      {
        const dof_id_type k = k_and_derivs.first;
        const std::vector<Real> derivs = k_and_derivs.second;
        for (unsigned pvar = 0; pvar < _dictator.numVariables(); ++pvar)
          _dkij_dvar[i][j][k][pvar] += derivs[pvar];
      }
    }
  }
  // gather the values of _du_dvar computed by different threads
  for (const auto & node_du : pfafc._du_dvar)
  {
    const dof_id_type i = node_du.first;
    if (!_du_dvar_computed_by_thread[i] && pfafc.getdU_dvarComputedByThread(i))
      _du_dvar[i] = node_du.second;
  }
}

const std::vector<Real> &
PorousFlowAdvectiveFluxCalculatorBase::getdU_dvar(dof_id_type node_i) const
{
  const auto & node_du = _du_dvar.find(node_i);
  if (node_du == _du_dvar.end())
    mooseError("PorousFlowAdvectiveFluxCalculatorBase UserObject " + name() +
               " _du_dvar does not contain node " + Moose::stringify(node_i));
  return node_du->second;
}

bool
PorousFlowAdvectiveFluxCalculatorBase::getdU_dvarComputedByThread(dof_id_type node_i) const
{
  const auto & node_du = _du_dvar_computed_by_thread.find(node_i);
  if (node_du == _du_dvar_computed_by_thread.end())
    mooseError("PorousFlowAdvectiveFluxCalculatorBase UserObject " + name() +
               " _du_dvar_computed_by_thread does not contain node " + Moose::stringify(node_i));
  return node_du->second;
}

const std::map<dof_id_type, std::vector<Real>> &
PorousFlowAdvectiveFluxCalculatorBase::getdK_dvar(dof_id_type node_i, dof_id_type node_j) const
{
  const auto & row_find = _dkij_dvar.find(node_i);
  if (row_find == _dkij_dvar.end())
    mooseError("PorousFlowAdvectiveFluxCalculatorBase UserObject " + name() +
               " _dkij_dvar does not contain node " + Moose::stringify(node_i));
  const std::map<dof_id_type, std::map<dof_id_type, std::vector<Real>>> & dkij_dvar_row =
      row_find->second;
  const auto & column_find = dkij_dvar_row.find(node_j);
  if (column_find == dkij_dvar_row.end())
    mooseError("PorousFlowAdvectiveFluxCalculatorBase UserObject " + name() +
               " _dkij_dvar on row " + Moose::stringify(node_i) + " does not contain node " +
               Moose::stringify(node_j));
  return column_find->second;
}

void
PorousFlowAdvectiveFluxCalculatorBase::getdFluxOut_dvars(
    std::map<dof_id_type, std::vector<Real>> & derivs, unsigned node_id) const
{
  derivs.clear();

  // the following will populate derivs[j] with j connected to i, and to nodes connected to i
  const std::map<dof_id_type, Real> dflux_out_du =
      AdvectiveFluxCalculatorBase::getdFluxOutdu(node_id);
  for (const auto & node_du : dflux_out_du)
  {
    const dof_id_type j = node_du.first;
    const Real dflux_out_du_j = node_du.second;
    derivs[j] = getdU_dvar(j);
    for (unsigned pvar = 0; pvar < _dictator.numVariables(); ++pvar)
      derivs[j][pvar] *= dflux_out_du_j;
  }

  // derivs is now sized correctly, because getdFluxOutdu(i) contains all nodes
  // connected to i and all nodes connected to nodes connected to i.  The
  // getdFluxOutdKij contains no extra nodes, so just += the dflux/dK terms to derivs
  const std::map<dof_id_type, std::map<dof_id_type, Real>> dflux_out_dKjk =
      AdvectiveFluxCalculatorBase::getdFluxOutdKjk(node_id);
  for (const auto & nodes : dflux_out_dKjk)
  {
    const dof_id_type j = nodes.first;
    for (const auto & node_du : nodes.second)
    {
      const dof_id_type k = node_du.first;
      const Real dflux_out_dK_jk = node_du.second;
      const std::map<dof_id_type, std::vector<Real>> dkj_dvarl = getdK_dvar(j, k);
      for (const auto & nodel_deriv : dkj_dvarl)
      {
        const dof_id_type l = nodel_deriv.first;
        for (unsigned pvar = 0; pvar < _dictator.numVariables(); ++pvar)
          derivs[l][pvar] += dflux_out_dK_jk * nodel_deriv.second[pvar];
      }
    }
  }
}
