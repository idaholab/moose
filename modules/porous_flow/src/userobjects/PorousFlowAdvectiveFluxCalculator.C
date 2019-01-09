//* This file is part of the MOOSE framework
//* https://www.mooseframework.org All rights reserved, see COPYRIGHT
//* for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT Licensed
//* under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAdvectiveFluxCalculator.h"

registerMooseObject("PorousFlowApp", PorousFlowAdvectiveFluxCalculator);

template <>
InputParameters
validParams<PorousFlowAdvectiveFluxCalculator>()
{
  InputParameters params = validParams<AdvectiveFluxCalculatorBase>();
  params.addClassDescription(
      "TODO "
      "Compute K_ij (a measure of advective flux from node i to node j) "
      "and R+ and R- (which quantify amount of antidiffusion to add) in the "
      "Kuzmin-Turek FEM-TVD multidimensional scheme.  Constant advective velocity is assumed");
  params.addRequiredParam<RealVectorValue>("gravity",
                                           "Gravitational acceleration vector downwards (m/s^2)");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addParam<unsigned int>(
      "fluid_component", 0, "The index corresponding to the fluid component for this UserObject");
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

PorousFlowAdvectiveFluxCalculator::PorousFlowAdvectiveFluxCalculator(
    const InputParameters & parameters)
  : AdvectiveFluxCalculatorBase(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _phase(getParam<unsigned int>("phase")),
    _permeability(getMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _dpermeability_dvar(
        getMaterialProperty<std::vector<RealTensorValue>>("dPorousFlow_permeability_qp_dvar")),
    _dpermeability_dgradvar(getMaterialProperty<std::vector<std::vector<RealTensorValue>>>(
        "dPorousFlow_permeability_qp_dgradvar")),
    _fluid_density_node(
        getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_nodal")),
    _dfluid_density_node_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_density_nodal_dvar")),
    _fluid_density_qp(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _dfluid_density_qp_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_fluid_phase_density_qp_dvar")),
    _fluid_viscosity(getMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_nodal")),
    _dfluid_viscosity_dvar(
        getMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_viscosity_nodal_dvar")),
    _pp(getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_nodal")),
    _grad_p(getMaterialProperty<std::vector<RealGradient>>("PorousFlow_grad_porepressure_qp")),
    _dgrad_p_dgrad_var(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_grad_porepressure_qp_dgradvar")),
    _dgrad_p_dvar(getMaterialProperty<std::vector<std::vector<RealGradient>>>(
        "dPorousFlow_grad_porepressure_qp_dvar")),
    _mass_fractions(
        getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")),
    _dmass_fractions_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
        "dPorousFlow_mass_frac_nodal_dvar")),
    _relative_permeability(
        getMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_nodal")),
    _drelative_permeability_dvar(getMaterialProperty<std::vector<std::vector<Real>>>(
        "dPorousFlow_relative_permeability_nodal_dvar")),
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

  if (_fluid_component >= _dictator.numComponents())
    paramError("fluid_component",
               "Fluid component number entered is greater than the number of fluid components "
               "specified in the Dictator. Remember that indexing starts at 0");

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
PorousFlowAdvectiveFluxCalculator::getInternodalVelocity(unsigned i, unsigned j, unsigned qp) const
{
  // The following is but one choice.
  // If you change this, you will probably have to change
  // - the derivative in executeOnElement
  // - getU
  // - dU_dvar
  return -_grad_phi[i][qp] *
         (_permeability[qp] * (_grad_p[qp][_phase] - _fluid_density_qp[qp][_phase] * _gravity)) *
         _phi[j][qp];
}

void
PorousFlowAdvectiveFluxCalculator::executeOnElement(
    dof_id_type global_i, dof_id_type global_j, unsigned local_i, unsigned local_j, unsigned qp)
{
  AdvectiveFluxCalculatorBase::executeOnElement(global_i, global_j, local_i, local_j, qp);
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

Real
PorousFlowAdvectiveFluxCalculator::getU(unsigned i) const
{
  // The following is but one choice.
  // If you change this, you will probably have to change
  // - getInternodalVelocity
  // - the derivative in executeOnElement
  // - dU_dvar
  return _mass_fractions[i][_phase][_fluid_component] * _fluid_density_node[i][_phase] *
         _relative_permeability[i][_phase] / _fluid_viscosity[i][_phase];
}

Real
PorousFlowAdvectiveFluxCalculator::dU_dvar(unsigned i, unsigned pvar) const
{
  Real du = _dmass_fractions_dvar[i][_phase][_fluid_component][pvar] *
            _fluid_density_node[i][_phase] * _relative_permeability[i][_phase] /
            _fluid_viscosity[i][_phase];
  du += _mass_fractions[i][_phase][_fluid_component] * _dfluid_density_node_dvar[i][_phase][pvar] *
        _relative_permeability[i][_phase] / _fluid_viscosity[i][_phase];
  du += _mass_fractions[i][_phase][_fluid_component] * _fluid_density_node[i][_phase] *
        _drelative_permeability_dvar[i][_phase][pvar] / _fluid_viscosity[i][_phase];
  du -= _mass_fractions[i][_phase][_fluid_component] * _fluid_density_node[i][_phase] *
        _relative_permeability[i][_phase] * _dfluid_viscosity_dvar[i][_phase][pvar] /
        std::pow(_fluid_viscosity[i][_phase], 2);
  return du;
}

void
PorousFlowAdvectiveFluxCalculator::timestepSetup()
{
  const bool resizing_was_needed =
      _resizing_needed; // _resizing_needed gets set to false at the end of
                        // AdvectiveFluxCalculatorBase::timestepSetup()
  AdvectiveFluxCalculatorBase::timestepSetup();
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
PorousFlowAdvectiveFluxCalculator::initialize()
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
PorousFlowAdvectiveFluxCalculator::execute()
{
  AdvectiveFluxCalculatorBase::execute();
  for (unsigned i = 0; i < _current_elem->n_nodes(); ++i)
  {
    const dof_id_type node_i = _current_elem->node_id(i);
    for (unsigned pvar = 0; pvar < _dictator.numVariables(); ++pvar)
      _du_dvar[node_i][pvar] = dU_dvar(i, pvar);
    _du_dvar_computed_by_thread[node_i] = true;
  }
}

void
PorousFlowAdvectiveFluxCalculator::threadJoin(const UserObject & uo)
{
  AdvectiveFluxCalculatorBase::threadJoin(uo);
  const PorousFlowAdvectiveFluxCalculator & pfafc =
      static_cast<const PorousFlowAdvectiveFluxCalculator &>(uo);
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
    if (!_du_dvar_computed_by_thread[i])
      _du_dvar[i] = node_du.second;
  }
}

const std::vector<Real> &
PorousFlowAdvectiveFluxCalculator::getdU_dvar(dof_id_type node_i) const
{
  const auto & node_du = _du_dvar.find(node_i);
  if (node_du == _du_dvar.end())
    mooseError("PorousFlowAdvectiveFluxCalculator UserObject " + name() +
               " _du_dvar does not contain node " + Moose::stringify(node_i));
  return node_du->second;
}

const std::map<dof_id_type, std::vector<Real>> &
PorousFlowAdvectiveFluxCalculator::getdK_dvar(dof_id_type node_i, dof_id_type node_j) const
{
  const auto & row_find = _dkij_dvar.find(node_i);
  if (row_find == _dkij_dvar.end())
    mooseError("PorousFlowAdvectiveFluxCalculator UserObject " + name() +
               " _dkij_dvar does not contain node " + Moose::stringify(node_i));
  const std::map<dof_id_type, std::map<dof_id_type, std::vector<Real>>> & dkij_dvar_row =
      row_find->second;
  const auto & column_find = dkij_dvar_row.find(node_j);
  if (column_find == dkij_dvar_row.end())
    mooseError("PorousFlowAdvectiveFluxCalculator UserObject " + name() + " _dkij_dvar on row " +
               Moose::stringify(node_i) + " does not contain node " + Moose::stringify(node_j));
  return column_find->second;
}

void
PorousFlowAdvectiveFluxCalculator::getdFluxOut_dvars(
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

  // derivs is populated correctly, so just add the dflux/dK terms
  const std::map<dof_id_type, std::map<dof_id_type, Real>> dflux_out_dKjk =
      AdvectiveFluxCalculatorBase::getdFluxOutdKjk(node_id);
  for (const auto & nodes : dflux_out_dKjk)
  {
    const dof_id_type j = nodes.first;
    for (const auto & node_du : nodes.second)
    {
      const dof_id_type k = node_du.first;
      const Real dflux_out_dK_jk = node_du.second;
      // const std::vector<Real> dkjk_dvar = getdK_dvar(j, k);
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
