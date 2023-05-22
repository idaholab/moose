//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDarcyBase.h"

#include "Assembly.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

InputParameters
PorousFlowDarcyBase::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<RealVectorValue>("gravity",
                                           "Gravitational acceleration vector downwards (m/s^2)");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addParam<unsigned>("full_upwind_threshold",
                            5,
                            "If, for each timestep, the number of "
                            "upwind-downwind swaps in an element is less than "
                            "this quantity, then full upwinding is used for that element.  "
                            "Otherwise the fallback scheme is employed.");
  MooseEnum fallback_enum("quick harmonic", "quick");
  params.addParam<MooseEnum>("fallback_scheme",
                             fallback_enum,
                             "quick: use nodal mobility without "
                             "preserving mass.  harmonic: use a "
                             "harmonic mean of nodal mobilities "
                             "and preserve fluid mass");
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
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_phases(_dictator.numPhases()),
    _gravity(getParam<RealVectorValue>("gravity")),
    _perm_derivs(_dictator.usePermDerivs()),
    _full_upwind_threshold(getParam<unsigned>("full_upwind_threshold")),
    _fallback_scheme(getParam<MooseEnum>("fallback_scheme").getEnum<FallbackEnum>()),
    _proto_flux(_num_phases),
    _jacobian(_num_phases),
    _num_upwinds(),
    _num_downwinds()
{
#ifdef LIBMESH_HAVE_TBB_API
  if (libMesh::n_threads() > 1)
    mooseWarning("PorousFlowDarcyBase: num_upwinds and num_downwinds may not be computed "
                 "accurately when using TBB and greater than 1 thread");
#endif
}

void
PorousFlowDarcyBase::timestepSetup()
{
  Kernel::timestepSetup();
  _num_upwinds = std::unordered_map<unsigned, std::vector<std::vector<unsigned>>>();
  _num_downwinds = std::unordered_map<unsigned, std::vector<std::vector<unsigned>>>();
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
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;

  const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);

  RealVectorValue deriv =
      _permeability[_qp] * (_grad_phi[_j][_qp] * _dgrad_p_dgrad_var[_qp][ph][pvar] -
                            _phi[_j][_qp] * _dfluid_density_qp_dvar[_qp][ph][pvar] * _gravity);

  deriv += _permeability[_qp] * (_dgrad_p_dvar[_qp][ph][pvar] * _phi[_j][_qp]);

  if (_perm_derivs)
  {
    deriv += _dpermeability_dvar[_qp][pvar] * _phi[_j][_qp] *
             (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity);
    for (const auto i : make_range(Moose::dim))
      deriv += _dpermeability_dgradvar[_qp][i][pvar] * _grad_phi[_j][_qp](i) *
               (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity);
  }

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
  computeResidualAndJacobian(JacRes::CALCULATE_RESIDUAL, 0.0);
}

void
PorousFlowDarcyBase::computeJacobian()
{
  computeResidualAndJacobian(JacRes::CALCULATE_JACOBIAN, _var.number());
}

void
PorousFlowDarcyBase::computeOffDiagJacobian(const unsigned int jvar)
{
  computeResidualAndJacobian(JacRes::CALCULATE_JACOBIAN, jvar);
}

void
PorousFlowDarcyBase::computeResidualAndJacobian(JacRes res_or_jac, unsigned int jvar)
{
  if ((res_or_jac == JacRes::CALCULATE_JACOBIAN) && _dictator.notPorousFlowVariable(jvar))
    return;

  // The PorousFlow variable index corresponding to the variable number jvar
  const unsigned int pvar =
      ((res_or_jac == JacRes::CALCULATE_JACOBIAN) ? _dictator.porousFlowVariableNum(jvar) : 0);

  prepareMatrixTag(_assembly, _var.number(), jvar);
  if ((_local_ke.n() == 0) && (res_or_jac == JacRes::CALCULATE_JACOBIAN)) // this removes a problem
                                                                          // encountered in the
                                                                          // initial timestep when
                                                                          // use_displaced_mesh=true
    return;

  // The number of nodes in the element
  const unsigned int num_nodes = _test.size();

  // Compute the residual and jacobian without the mobility terms. Even if we are computing the
  // Jacobian we still need this in order to see which nodes are upwind and which are downwind.
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    _proto_flux[ph].assign(num_nodes, 0);
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      for (_i = 0; _i < num_nodes; ++_i)
        _proto_flux[ph][_i] += _JxW[_qp] * _coord[_qp] * darcyQp(ph);
    }
  }

  // for this element, record whether each node is "upwind" or "downwind" (or neither)
  const unsigned elem = _current_elem->id();
  if (_num_upwinds.find(elem) == _num_upwinds.end())
  {
    _num_upwinds[elem] = std::vector<std::vector<unsigned>>(_num_phases);
    _num_downwinds[elem] = std::vector<std::vector<unsigned>>(_num_phases);
    for (unsigned ph = 0; ph < _num_phases; ++ph)
    {
      _num_upwinds[elem][ph].assign(num_nodes, 0);
      _num_downwinds[elem][ph].assign(num_nodes, 0);
    }
  }
  // record the information once per nonlinear iteration
  if (res_or_jac == JacRes::CALCULATE_JACOBIAN && jvar == _var.number())
  {
    for (unsigned ph = 0; ph < _num_phases; ++ph)
    {
      for (unsigned nod = 0; nod < num_nodes; ++nod)
      {
        if (_proto_flux[ph][nod] > 0)
          _num_upwinds[elem][ph][nod]++;
        else if (_proto_flux[ph][nod] < 0)
          _num_downwinds[elem][ph][nod]++;
      }
    }
  }

  // based on _num_upwinds and _num_downwinds, calculate the maximum number
  // of upwind-downwind swaps that have been encountered in this timestep
  // for this element
  std::vector<unsigned> max_swaps(_num_phases, 0);
  for (unsigned ph = 0; ph < _num_phases; ++ph)
  {
    for (unsigned nod = 0; nod < num_nodes; ++nod)
      max_swaps[ph] = std::max(
          max_swaps[ph], std::min(_num_upwinds[elem][ph][nod], _num_downwinds[elem][ph][nod]));
  }

  // size the _jacobian correctly and calculate it for the case residual = _proto_flux
  if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
  {
    for (unsigned ph = 0; ph < _num_phases; ++ph)
    {
      _jacobian[ph].resize(_local_ke.m());
      for (_i = 0; _i < _test.size(); _i++)
      {
        _jacobian[ph][_i].assign(_local_ke.n(), 0.0);
        for (_j = 0; _j < _phi.size(); _j++)
          for (_qp = 0; _qp < _qrule->n_points(); _qp++)
            _jacobian[ph][_i][_j] += _JxW[_qp] * _coord[_qp] * darcyQpJacobian(jvar, ph);
      }
    }
  }

  // Loop over all the phases, computing the mass flux, which
  // gets placed into _proto_flux, and the derivative of this
  // which gets placed into _jacobian
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    if (max_swaps[ph] < _full_upwind_threshold)
      fullyUpwind(res_or_jac, ph, pvar);
    else
    {
      switch (_fallback_scheme)
      {
        case FallbackEnum::QUICK:
          quickUpwind(res_or_jac, ph, pvar);
          break;
        case FallbackEnum::HARMONIC:
          harmonicMean(res_or_jac, ph, pvar);
          break;
      }
    }
  }

  // Add results to the Residual or Jacobian
  if (res_or_jac == JacRes::CALCULATE_RESIDUAL)
  {
    prepareVectorTag(_assembly, _var.number());
    for (_i = 0; _i < _test.size(); _i++)
      for (unsigned int ph = 0; ph < _num_phases; ++ph)
        _local_re(_i) += _proto_flux[ph][_i];
    accumulateTaggedLocalResidual();

    if (_has_save_in)
      for (unsigned int i = 0; i < _save_in.size(); i++)
        _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }

  if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
  {
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        for (unsigned int ph = 0; ph < _num_phases; ++ph)
          _local_ke(_i, _j) += _jacobian[ph][_i][_j];

    accumulateTaggedLocalMatrix();

    if (_has_diag_save_in && jvar == _var.number())
    {
      unsigned int rows = _local_ke.m();
      DenseVector<Number> diag(rows);
      for (unsigned int i = 0; i < rows; i++)
        diag(i) = _local_ke(i, i);

      for (unsigned int i = 0; i < _diag_save_in.size(); i++)
        _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
    }
  }
}

void
PorousFlowDarcyBase::fullyUpwind(JacRes res_or_jac, unsigned int ph, unsigned int pvar)
{
  /**
   * Perform the full upwinding by multiplying the residuals at the upstream nodes by their
   * mobilities.
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
   * which was done in the _proto_flux calculation above.
   *
   * NOTE: Physically _proto_flux[i][ph] is a measure of fluid of phase ph flowing out of node i.
   * If we had left in mobility, it would be exactly the component mass flux flowing out of node
   *i.
   *
   * This leads to the definition of upwinding:
   *
   * If _proto_flux(i)[ph] is positive then we use mobility_i.  That is we use the upwind value of
   * mobility.
   *
   * The final subtle thing is we must also conserve fluid mass: the total component mass flowing
   * out of node i must be the sum of the masses flowing into the other nodes.
   **/

  // The number of nodes in the element
  const unsigned int num_nodes = _test.size();

  Real mob;
  Real dmob;
  // Define variables used to ensure mass conservation
  Real total_mass_out = 0.0;
  Real total_in = 0.0;

  // The following holds derivatives of these
  std::vector<Real> dtotal_mass_out;
  std::vector<Real> dtotal_in;
  if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
  {
    dtotal_mass_out.assign(num_nodes, 0.0);
    dtotal_in.assign(num_nodes, 0.0);
  }

  // Perform the upwinding using the mobility
  std::vector<bool> upwind_node(num_nodes);
  for (unsigned int n = 0; n < num_nodes; ++n)
  {
    if (_proto_flux[ph][n] >= 0.0) // upstream node
    {
      upwind_node[n] = true;
      // The mobility at the upstream node
      mob = mobility(n, ph);
      if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
      {
        // The derivative of the mobility wrt the PorousFlow variable
        dmob = dmobility(n, ph, pvar);

        for (_j = 0; _j < _phi.size(); _j++)
          _jacobian[ph][n][_j] *= mob;

        if (_test.size() == _phi.size())
          /* mobility at node=n depends only on the variables at node=n, by construction.  For
           * linear-lagrange variables, this means that Jacobian entries involving the derivative
           * of mobility will only be nonzero for derivatives wrt variables at node=n.  Hence the
           * [n][n] in the line below.  However, for other variable types (eg constant monomials)
           * I cannot tell what variable number contributes to the derivative.  However, in all
           * cases I can possibly imagine, the derivative is zero anyway, since in the full
           * upwinding scheme, mobility shouldn't depend on these other sorts of variables.
           */
          _jacobian[ph][n][n] += dmob * _proto_flux[ph][n];

        for (_j = 0; _j < _phi.size(); _j++)
          dtotal_mass_out[_j] += _jacobian[ph][n][_j];
      }
      _proto_flux[ph][n] *= mob;
      total_mass_out += _proto_flux[ph][n];
    }
    else
    {
      upwind_node[n] = false;
      total_in -= _proto_flux[ph][n]; /// note the -= means the result is positive
      if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
        for (_j = 0; _j < _phi.size(); _j++)
          dtotal_in[_j] -= _jacobian[ph][n][_j];
    }
  }

  // Conserve mass over all phases by proportioning the total_mass_out mass to the inflow nodes,
  // weighted by their proto_flux values
  for (unsigned int n = 0; n < num_nodes; ++n)
  {
    if (!upwind_node[n]) // downstream node
    {
      if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
        for (_j = 0; _j < _phi.size(); _j++)
        {
          _jacobian[ph][n][_j] *= total_mass_out / total_in;
          _jacobian[ph][n][_j] +=
              _proto_flux[ph][n] * (dtotal_mass_out[_j] / total_in -
                                    dtotal_in[_j] * total_mass_out / total_in / total_in);
        }
      _proto_flux[ph][n] *= total_mass_out / total_in;
    }
  }
}

void
PorousFlowDarcyBase::quickUpwind(JacRes res_or_jac, unsigned int ph, unsigned int pvar)
{
  // The number of nodes in the element
  const unsigned int num_nodes = _test.size();

  Real mob;
  Real dmob;

  // Use the raw nodal mobility
  for (unsigned int n = 0; n < num_nodes; ++n)
  {
    // The mobility at the node
    mob = mobility(n, ph);
    if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
    {
      // The derivative of the mobility wrt the PorousFlow variable
      dmob = dmobility(n, ph, pvar);

      for (_j = 0; _j < _phi.size(); _j++)
        _jacobian[ph][n][_j] *= mob;

      if (_test.size() == _phi.size())
        /* mobility at node=n depends only on the variables at node=n, by construction.  For
         * linear-lagrange variables, this means that Jacobian entries involving the derivative
         * of mobility will only be nonzero for derivatives wrt variables at node=n.  Hence the
         * [n][n] in the line below.  However, for other variable types (eg constant monomials)
         * I cannot tell what variable number contributes to the derivative.  However, in all
         * cases I can possibly imagine, the derivative is zero anyway, since in the full
         * upwinding scheme, mobility shouldn't depend on these other sorts of variables.
         */
        _jacobian[ph][n][n] += dmob * _proto_flux[ph][n];
    }
    _proto_flux[ph][n] *= mob;
  }
}

void
PorousFlowDarcyBase::harmonicMean(JacRes res_or_jac, unsigned int ph, unsigned int pvar)
{
  // The number of nodes in the element
  const unsigned int num_nodes = _test.size();

  std::vector<Real> mob(num_nodes);
  unsigned num_zero = 0;
  unsigned zero_mobility_node = std::numeric_limits<unsigned>::max();
  Real harmonic_mob = 0;
  for (unsigned n = 0; n < num_nodes; ++n)
  {
    mob[n] = mobility(n, ph);
    if (mob[n] == 0.0)
    {
      zero_mobility_node = n;
      num_zero++;
    }
    else
      harmonic_mob += 1.0 / mob[n];
  }
  if (num_zero > 0)
    harmonic_mob = 0.0;
  else
    harmonic_mob = (1.0 * num_nodes) / harmonic_mob;

  // d(harmonic_mob)/d(PorousFlow variable at node n)
  std::vector<Real> dharmonic_mob(num_nodes, 0.0);
  if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
  {
    const Real harm2 = std::pow(harmonic_mob, 2) / (1.0 * num_nodes);
    if (num_zero == 0)
      for (unsigned n = 0; n < num_nodes; ++n)
        dharmonic_mob[n] = dmobility(n, ph, pvar) * harm2 / std::pow(mob[n], 2);
    else if (num_zero == 1)
      dharmonic_mob[zero_mobility_node] =
          num_nodes * dmobility(zero_mobility_node, ph, pvar); // other derivs are zero
    // if num_zero > 1 then all dharmonic_mob = 0.0
  }

  if (res_or_jac == JacRes::CALCULATE_JACOBIAN)
    for (unsigned n = 0; n < num_nodes; ++n)
      for (_j = 0; _j < _phi.size(); _j++)
      {
        _jacobian[ph][n][_j] *= harmonic_mob;
        if (_test.size() == _phi.size())
          _jacobian[ph][n][_j] += dharmonic_mob[_j] * _proto_flux[ph][n];
      }

  if (res_or_jac == JacRes::CALCULATE_RESIDUAL)
    for (unsigned n = 0; n < num_nodes; ++n)
      _proto_flux[ph][n] *= harmonic_mob;
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
