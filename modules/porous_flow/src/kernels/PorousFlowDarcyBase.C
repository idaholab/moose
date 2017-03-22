/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowDarcyBase.h"

#include "Assembly.h"
#include "MooseMesh.h"
#include "SystemBase.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<PorousFlowDarcyBase>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<RealVectorValue>("gravity",
                                           "Gravitational acceleration vector downwards (m/s^2)");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addClassDescription("Fully-upwinded advective Darcy flux");
  return params;
}

PorousFlowDarcyBase::PorousFlowDarcyBase(const InputParameters & parameters)
  : Kernel(parameters),
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
    _porousflow_dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_phases(_porousflow_dictator.numPhases()),
    _gravity(getParam<RealVectorValue>("gravity"))
{
}

Real
PorousFlowDarcyBase::darcyQp(unsigned int ph) const
{
  return _grad_test[_i][_qp] *
         (_permeability[_qp] * (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity));
}

Real
PorousFlowDarcyBase::darcyQpJacobian(unsigned int jvar, unsigned int ph) const
{
  if (_porousflow_dictator.notPorousFlowVariable(jvar))
    return 0.0;

  const unsigned int pvar = _porousflow_dictator.porousFlowVariableNum(jvar);
  RealVectorValue deriv = _dpermeability_dvar[_qp][pvar] * _phi[_j][_qp] *
                          (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity);
  for (unsigned i = 0; i < LIBMESH_DIM; ++i)
    deriv += _dpermeability_dgradvar[_qp][i][pvar] * _grad_phi[_j][_qp](i) *
             (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity);
  deriv += _permeability[_qp] * (_grad_phi[_j][_qp] * _dgrad_p_dgrad_var[_qp][ph][pvar] -
                                 _phi[_j][_qp] * _dfluid_density_qp_dvar[_qp][ph][pvar] * _gravity);
  deriv += _permeability[_qp] * (_dgrad_p_dvar[_qp][ph][pvar] * _phi[_j][_qp]);
  return _grad_test[_i][_qp] * deriv;
}

Real
PorousFlowDarcyBase::computeQpResidual()
{
  mooseError("PorousFlowDarcyBase: computeQpResidual called");
  return 0.0;
}

void
PorousFlowDarcyBase::computeResidual()
{
  upwind(CALCULATE_RESIDUAL, 0.0);
}

void
PorousFlowDarcyBase::computeJacobian()
{
  upwind(CALCULATE_JACOBIAN, _var.number());
}

void
PorousFlowDarcyBase::computeOffDiagJacobian(unsigned int jvar)
{
  upwind(CALCULATE_JACOBIAN, jvar);
}

void
PorousFlowDarcyBase::upwind(JacRes res_or_jac, unsigned int jvar)
{
  if ((res_or_jac == CALCULATE_JACOBIAN) && _porousflow_dictator.notPorousFlowVariable(jvar))
    return;

  /// The PorousFlow variable index corresponding to the variable number jvar
  const unsigned int pvar =
      ((res_or_jac == CALCULATE_JACOBIAN) ? _porousflow_dictator.porousFlowVariableNum(jvar) : 0);

  /// The number of nodes in the element
  const unsigned int num_nodes = _test.size();

  /// Compute the residual and jacobian without the mobility terms. Even if we are computing the Jacobian
  /// we still need this in order to see which nodes are upwind and which are downwind.

  std::vector<std::vector<Real>> component_re(num_nodes);
  for (_i = 0; _i < num_nodes; ++_i)
  {
    component_re[_i].assign(_num_phases, 0.0);
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (unsigned ph = 0; ph < _num_phases; ++ph)
        component_re[_i][ph] += _JxW[_qp] * _coord[_qp] * darcyQp(ph);
  }

  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);
  if ((ke.n() == 0) && (res_or_jac == CALCULATE_JACOBIAN)) // this removes a problem encountered in
                                                           // the initial timestep when
                                                           // use_displaced_mesh=true
    return;

  std::vector<std::vector<std::vector<Real>>> component_ke;
  if (res_or_jac == CALCULATE_JACOBIAN)
  {
    component_ke.resize(ke.m());
    for (_i = 0; _i < _test.size(); _i++)
    {
      component_ke[_i].resize(ke.n());
      for (_j = 0; _j < _phi.size(); _j++)
      {
        component_ke[_i][_j].resize(_num_phases);
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          for (unsigned ph = 0; ph < _num_phases; ++ph)
            component_ke[_i][_j][ph] += _JxW[_qp] * _coord[_qp] * darcyQpJacobian(jvar, ph);
      }
    }
  }

  /**
   * Now perform the upwinding by multiplying the residuals at the upstream nodes by their
   *mobilities.
   * Mobility is different for each phase, and in each situation:
   *   mobility = density / viscosity    for single-component Darcy flow
   *   mobility = mass_fraction * density * relative_perm / viscosity    for multi-component,
   *multiphase flow
   *   mobility = enthalpy * density * relative_perm / viscosity    for heat convection
   *
   * The residual for the kernel is the sum over Darcy fluxes for each phase.
   * The Darcy flux for a particular phase is
   * R_i = int{mobility*flux_no_mob} = int{mobility*grad(pot)*permeability*grad(test_i)}
   * for node i.  where int is the integral over the element.
   * However, in fully-upwind, the first step is to take the mobility outside the integral,
   * which was done in the _component_re calculation above.
   *
   * NOTE: Physically _component_re[i][ph] is a measure of fluid of phase ph flowing out of node i.
   * If we had left in mobility, it would be exactly the component mass flux flowing out of node i.
   *
   * This leads to the definition of upwinding:
   *
   * If _component_re(i)[ph] is positive then we use mobility_i.  That is we use the upwind value of
   *mobility.
   *
   * The final subtle thing is we must also conserve fluid mass: the total component mass flowing
   *out of node i
   * must be the sum of the masses flowing into the other nodes.
  **/

  // Following are used below in steady-state calculations
  Real knorm = 0.0;
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    knorm += _permeability[qp].tr();
  const Real lsq = _grad_test[0][0] * _grad_test[0][0];
  const unsigned int dim = _mesh.dimension();
  const Real l2md = std::pow(lsq, 0.5 * (2.0 - dim));
  const Real l1md = std::pow(lsq, 0.5 * (1.0 - dim));

  /// Loop over all the phases
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {

    // FIRST:
    // this is a dirty way of getting around precision loss problems
    // and problems at steadystate where upwinding oscillates from
    // node-to-node causing nonconvergence.
    // The residual = int_{ele}*grad_test*k*(gradP - rho*g) = L^(d-1)*k*(gradP - rho*g), where d is
    // dimension
    // I assume that if one nodal P changes by P*1E-8 then this is
    // a "negligible" change.  The residual will change by L^(d-2)*k*P*1E-8
    // Similarly if rho*g changes by rho*g*1E-8 then this is a "negligible change"
    // Hence the formulae below, with grad_test = 1/L
    Real pnorm = 0.0;
    Real gravnorm = 0.0;
    for (unsigned int n = 0; n < num_nodes; ++n)
    {
      pnorm += _pp[n][ph] * _pp[n][ph];
      gravnorm += _fluid_density_node[n][ph] * _fluid_density_node[n][ph];
    }
    gravnorm *= _gravity * _gravity;
    const Real cutoff = 1.0E-8 * knorm * (std::sqrt(pnorm) * l2md + std::sqrt(gravnorm) * l1md);
    bool reached_steady = true;
    for (unsigned int nodenum = 0; nodenum < num_nodes; ++nodenum)
    {
      if (component_re[nodenum][ph] >= cutoff)
      {
        reached_steady = false;
        break;
      }
    }

    Real mob;
    Real dmob;
    /// Define variables used to ensure mass conservation
    Real total_mass_out = 0.0;
    Real total_in = 0.0;

    /// The following holds derivatives of these
    std::vector<Real> dtotal_mass_out;
    std::vector<Real> dtotal_in;
    if (res_or_jac == CALCULATE_JACOBIAN)
    {
      dtotal_mass_out.resize(num_nodes);
      dtotal_in.resize(num_nodes);

      for (unsigned int n = 0; n < num_nodes; ++n)
      {
        dtotal_mass_out[n] = 0.0;
        dtotal_in[n] = 0.0;
      }
    }

    /// Perform the upwinding using the mobility
    std::vector<bool> upwind_node(num_nodes);
    for (unsigned int n = 0; n < num_nodes; ++n)
    {
      if (component_re[n][ph] >= cutoff || reached_steady) // upstream node
      {
        upwind_node[n] = true;
        /// The mobility at the upstream node
        mob = mobility(n, ph);
        if (res_or_jac == CALCULATE_JACOBIAN)
        {
          /// The derivative of the mobility wrt the PorousFlow variable
          dmob = dmobility(n, ph, pvar);

          for (_j = 0; _j < _phi.size(); _j++)
            component_ke[n][_j][ph] *= mob;

          if (_test.size() == _phi.size())
            /* mobility at node=n depends only on the variables at node=n, by construction.  For
             * linear-lagrange variables, this means that Jacobian entries involving the derivative
             * of mobility will only be nonzero for derivatives wrt variables at node=n.  Hence the
             * [n][n] in the line below.  However, for other variable types (eg constant monomials)
             * I cannot tell what variable number contributes to the derivative.  However, in all
             * cases I can possibly imagine, the derivative is zero anyway, since in the full
             * upwinding scheme, mobility shouldn't depend on these other sorts of variables.
             */
            component_ke[n][n][ph] += dmob * component_re[n][ph];

          for (_j = 0; _j < _phi.size(); _j++)
            dtotal_mass_out[_j] += component_ke[n][_j][ph];
        }
        component_re[n][ph] *= mob;
        total_mass_out += component_re[n][ph];
      }
      else
      {
        upwind_node[n] = false;
        total_in -= component_re[n][ph]; /// note the -= means the result is positive
        if (res_or_jac == CALCULATE_JACOBIAN)
          for (_j = 0; _j < _phi.size(); _j++)
            dtotal_in[_j] -= component_ke[n][_j][ph];
      }
    }

    /// Conserve mass over all phases by proportioning the total_mass_out mass to the inflow nodes, weighted by their component_re values
    if (!reached_steady)
    {
      for (unsigned int n = 0; n < num_nodes; ++n)
      {
        if (!upwind_node[n]) // downstream node
        {
          if (res_or_jac == CALCULATE_JACOBIAN)
            for (_j = 0; _j < _phi.size(); _j++)
            {
              component_ke[n][_j][ph] *= total_mass_out / total_in;
              component_ke[n][_j][ph] +=
                  component_re[n][ph] * (dtotal_mass_out[_j] / total_in -
                                         dtotal_in[_j] * total_mass_out / total_in / total_in);
            }
          component_re[n][ph] *= total_mass_out / total_in;
        }
      }
    }
  }

  /// Add results to the Residual or Jacobian
  if (res_or_jac == CALCULATE_RESIDUAL)
  {
    DenseVector<Number> & re = _assembly.residualBlock(_var.number());

    _local_re.resize(re.size());
    _local_re.zero();
    for (_i = 0; _i < _test.size(); _i++)
      for (unsigned int ph = 0; ph < _num_phases; ++ph)
        _local_re(_i) += component_re[_i][ph];

    re += _local_re;

    if (_has_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i = 0; i < _save_in.size(); i++)
        _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
    }
  }

  if (res_or_jac == CALCULATE_JACOBIAN)
  {
    _local_ke.resize(ke.m(), ke.n());
    _local_ke.zero();

    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        for (unsigned int ph = 0; ph < _num_phases; ++ph)
          _local_ke(_i, _j) += component_ke[_i][_j][ph];

    ke += _local_ke;

    if (_has_diag_save_in && jvar == _var.number())
    {
      unsigned int rows = ke.m();
      DenseVector<Number> diag(rows);
      for (unsigned int i = 0; i < rows; i++)
        diag(i) = _local_ke(i, i);

      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i = 0; i < _diag_save_in.size(); i++)
        _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
    }
  }
}

Real
PorousFlowDarcyBase::mobility(unsigned nodenum, unsigned phase) const
{
  return _fluid_density_node[nodenum][phase] / _fluid_viscosity[nodenum][phase];
}

Real
PorousFlowDarcyBase::dmobility(unsigned nodenum, unsigned phase, unsigned pvar) const
{
  Real dm = _dfluid_density_node_dvar[nodenum][phase][pvar] / _fluid_viscosity[nodenum][phase];
  dm -= _fluid_density_node[nodenum][phase] * _dfluid_viscosity_dvar[nodenum][phase][pvar] /
        std::pow(_fluid_viscosity[nodenum][phase], 2);
  return dm;
}
