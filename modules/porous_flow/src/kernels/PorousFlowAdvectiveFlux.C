/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowAdvectiveFlux.h"
#include "Assembly.h"
// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<PorousFlowAdvectiveFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<unsigned int>("component_index", 0, "The index corresponding to the component for this kernel");
  params.addRequiredParam<RealVectorValue>("gravity", "Gravitational acceleration vector downwards (m/s^2)");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of PorousFlow variable names");
  params.addClassDescription("Fully-upwinded advective flux of the component given by component_index");
  return params;
}

PorousFlowAdvectiveFlux::PorousFlowAdvectiveFlux(const InputParameters & parameters) :
  Kernel(parameters),
  _permeability(getMaterialProperty<RealTensorValue>("PorousFlow_permeability")),
  _dpermeability_dvar(getMaterialProperty<std::vector<RealTensorValue> >("dPorousFlow_permeability_dvar")),
  _fluid_density_node(getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density")),
  _dfluid_density_node_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_fluid_phase_density_dvar")),
  _fluid_density_qp(getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density_qp")),
  _dfluid_density_qp_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_fluid_phase_density_qp_dvar")),
  _fluid_viscosity(getMaterialProperty<std::vector<Real> >("PorousFlow_viscosity")),
  _dfluid_viscosity_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_viscosity_dvar")),
  _mass_fractions(getMaterialProperty<std::vector<std::vector<Real> > >("PorousFlow_mass_frac")),
  _dmass_fractions_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real> > > >("dPorousFlow_mass_frac_dvar")),
  _grad_p(getMaterialProperty<std::vector<RealGradient> >("PorousFlow_grad_porepressure")),
  _dgrad_p_dgrad_var(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_grad_porepressure_dgradvar")),
  _relative_permeability(getMaterialProperty<std::vector<Real> >("PorousFlow_relative_permeability")),
  _drelative_permeability_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_relative_permeability_dvar")),
  _porousflow_dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
  _component_index(getParam<unsigned int>("component_index")),
  _gravity(getParam<RealVectorValue>("gravity"))
{
  // Make sure that this kernels variable is a valid PorousFlow variable
  if (_porousflow_dictator_UO.not_porflow_var(_var.number()))
    mooseError(" Variable " << _var.name() << " in the " << _name << " kernel is not a valid PorousFlow variable");

  _num_phases = _porousflow_dictator_UO.num_phases();
}

Real
PorousFlowAdvectiveFlux::darcyQp(unsigned int ph)
{
  return _grad_test[_i][_qp]*(_permeability[_qp]*(_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph]*_gravity));
}

Real PorousFlowAdvectiveFlux::darcyQpJacobian(unsigned int jvar, unsigned int ph)
{
  if (_porousflow_dictator_UO.not_porflow_var(jvar))
    return 0.0;

  const unsigned int pvar = _porousflow_dictator_UO.porflow_var_num(jvar);
  return _grad_test[_i][_qp] * (_dpermeability_dvar[_qp][pvar] * (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph]*_gravity) + _permeability[_qp] * (_grad_phi[_j][_qp] * _dgrad_p_dgrad_var[_qp][ph][pvar] - _phi[_j][_qp] * _dfluid_density_qp_dvar[_qp][ph][pvar] * _gravity) );
}



Real PorousFlowAdvectiveFlux::computeQpResidual()
{
  RealVectorValue qpresidual = 0.0;

  for (unsigned ph = 0; ph < _num_phases; ++ph)
    qpresidual += _mass_fractions[_qp][ph][_component_index] * (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity);

  return _grad_test[_i][_qp] * (_permeability[_qp] * qpresidual);
}

void PorousFlowAdvectiveFlux::computeResidual()
{
  upwind(true, false, 0.);
}


Real PorousFlowAdvectiveFlux::computeQpJac(unsigned int pvar)
{
  RealVectorValue qpjacobian = 0.0;

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    qpjacobian += _phi[_j][_qp] * _dmass_fractions_dvar[_qp][ph][_component_index][pvar] * (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity);
    qpjacobian += _mass_fractions[_qp][ph][_component_index] * (_grad_phi[_j][_qp] * _dgrad_p_dgrad_var[_qp][ph][pvar] - _phi[_j][_qp] * _dfluid_density_qp_dvar[_qp][ph][pvar] * _gravity);
  }

  return _grad_test[_i][_qp] * (_permeability[_qp] * qpjacobian);
}

Real PorousFlowAdvectiveFlux::computeQpJacobian()
{
  if (_porousflow_dictator_UO.not_porflow_var(_var.number()))
    return 0.0;
  return computeQpJac(_porousflow_dictator_UO.porflow_var_num(_var.number()));;
}

void PorousFlowAdvectiveFlux::computeJacobian()
{
   upwind(false, true, _var.number());
}

Real PorousFlowAdvectiveFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_porousflow_dictator_UO.not_porflow_var(jvar))
    return 0.0;
  return computeQpJac(_porousflow_dictator_UO.porflow_var_num(jvar));;
}

void PorousFlowAdvectiveFlux::computeOffDiagJacobian(unsigned int jvar)
{
   upwind(false, true, jvar);
}


void PorousFlowAdvectiveFlux::upwind(bool compute_res, bool compute_jac, unsigned int jvar)
{
  if (compute_jac && _porousflow_dictator_UO.not_porflow_var(jvar))
      return;

  /// The PorousFlow variable index corresponding to the variable number jvar
  const unsigned int pvar = (compute_jac ? _porousflow_dictator_UO.porflow_var_num(jvar) : 0);

  /// The number of nodes in the element
  const unsigned int num_nodes = _test.size();

  /// Compute the residual and jacobian without the mobility terms. Even if we are computing the Jacobian
  /// we still need this in order to see which nodes are upwind and which are downwind.

  std::vector<std::vector<Real> > component_re(num_nodes, _num_phases);
  for (unsigned i = 0; i < num_nodes; ++i)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (unsigned ph = 0; ph < _num_phases; ++ph)
	component_re[i][ph] += _JxW[_qp] * _coord[_qp] * darcyQp(ph);
  
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);

  std::vector<std::vector<std::vector<Real> > > component_ke(ke.m(), ke.n(), _num_phases);
  if (compute_jac)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
	for (_qp = 0; _qp < _qrule->n_points(); _qp++)
	  for (unsigned ph = 0; ph < _num_phases; ++ph)
	    component_ke[_i][_j][ph] += _JxW[_qp] * _coord[_qp] * darcyQpJacobian(jvar, ph);

  /**
   * Now perform the upwinding by multiplying the residuals at the upstream nodes by their mobilities
   * The residual for the kernel is the sum over Darcy fluxes for each phase.
   * The Darcy flux for a particular phase is
   * R_i = int{massfrac*mobility*flux_no_mob} = int{massfrac*mobility*grad(pot)*permeability*grad(test_i)}
   * for node i.  where int is the integral over the element.
   * However, in fully-upwind, the first step is to take the massfrac*mobility outside the integral,
   * which was done in the _component_re calculation above.
   *
   * NOTE: Physically _component_re[i][ph] is a measure of fluid of phase ph flowing out of node i.
   * If we had left in massfrac*mobility, it would be exactly the component mass flux flowing out of node i.
   *
   * This leads to the definition of upwinding:
   *
   * If _component_re(i)[ph] is positive then we use mass_frac_i*mobility_i.  That is we use the upwind value of mobility.
   *
   * The final subtle thing is we must also conserve fluid mass: the total component mass flowing out of node i
   * must be the sum of the masses flowing into the other nodes.
  **/

  /// Loop over all the phases
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {

    // FIRST:
    // this is a dirty way of getting around precision loss problems
    // and problems at steadystate where upwinding oscillates from
    // node-to-node causing nonconvergence.
    // I'm not sure if i actually need to do this in moose.  Certainly
    // in cosflow it is necessary.
    // I will code a better algorithm if necessary
    bool reached_steady = true;
    for (unsigned int nodenum = 0; nodenum < num_nodes ; ++nodenum)
    {
      if (component_re[nodenum][ph] >= 1E-20)
      {
	reached_steady = false;
	break;
      }
    }

    Real mobility;
    Real dmobility;
    /// Define variables used to ensure mass conservation
    Real total_mass_out = 0;
    Real total_in = 0;

    /// The following holds derivatives of these
    std::vector<Real> dtotal_mass_out;
    std::vector<Real> dtotal_in;
    if (compute_jac)
    {
      dtotal_mass_out.resize(num_nodes);
      dtotal_in.resize(num_nodes);

      for (unsigned int n = 0; n < num_nodes ; ++n)
      {
        dtotal_mass_out[n] = 0;
        dtotal_in[n] = 0;
      }
    }

    /// Perform the upwinding using the mass_frac*mobility (massfrac * relative permeability * fluid density / fluid viscosity)
    for (unsigned int n = 0; n < num_nodes ; ++n)
      {
        if (component_re[n][ph] >= 0) // upstream node
        {
          /// The massfrac*mobility at the upstream node
          mobility = _mass_fractions[n][ph][_component_index] * _fluid_density_node[n][ph] * _relative_permeability[n][ph] / _fluid_viscosity[n][ph];

          if (compute_jac)
          {
            /// The derivative of the massfrac*mobility wrt the PorousFlow variable
            dmobility = _dmass_fractions_dvar[n][ph][_component_index][pvar] * _fluid_density_node[n][ph] * _relative_permeability[n][ph] / _fluid_viscosity[n][ph];
            dmobility += mass_fractions[n][ph][_component_index] * _dfluid_density_node[n][ph][pvar] * _relative_permeability[n][ph] / _fluid_viscosity[n][ph];
            dmobility += mass_fractions[n][ph][_component_index] * _fluid_density_node[n][ph] * _drelative_permeability_dvar[n][ph][pvar] / _fluid_viscosity[n][ph];
            dmobility -= mobility / _fluid_visocity[n][ph] * _dfluid_viscosity_dvar[n][ph][pvar];

            for (_j = 0; _j < _phi.size(); _j++)
              component_ke[n][_j][ph] *= mobility;

            component_ke[n][n][ph] += dmobility * component_re[n][ph];

            for (_j = 0; _j < _phi.size(); _j++)
              dtotal_mass_out[_j] += component_ke[n][_j][ph];
          }
          component_re[n][ph] *= mobility;
          total_mass_out += component_re[n][ph];
        }
        else
        {
          total_in -= component_re[n][ph]; /// note the -= means the result is positive
          if (compute_jac)
            for (_j = 0; _j < _phi.size(); _j++)
              dtotal_in[_j] -= component_ke[n][_j][ph];
        }
      }

    /// Conserve mass over all phases by proportioning the total_mass_out mass to the inflow nodes, weighted by their component_re values
    if (!reached_steady)
    {
      for (unsigned int n = 0; n < num_nodes; ++n)
      {
        if (component_re[n][ph] < 0)
        {
          if (compute_jac)
            for (_j = 0; _j < _phi.size(); _j++)
            {
              component_ke[n][_j][ph] *= total_mass_out / total_in;
              component_ke[n][_j][ph] += component_re[n][ph] * (dtotal_mass_out[_j] / total_in - dtotal_in[_j] * total_mass_out / total_in / total_in);
            }
          component_re[n][ph] *= total_mass_out / total_in;
        }
      }
    }
  }
  // to here

  /// Compute the residual and jacobian without the mobility terms. Even if we are computing the Jacobian
  /// we still need this in order to see which nodes are upwind and which are downwind.
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  /// Form the _local_re contribution to the residual without the mobility term
 for (_i = 0; _i < _test.size(); _i++)
   for (unsigned int ph = 0; ph < _num_phases; ++ph)
     _local_re(_i) += component_re[_i][ph];

  /// Add results to the Residual or Jacobian
  if (compute_res)
  {
    re += _local_re;

    if (_has_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i = 0; i < _save_in.size(); i++)
        _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
    }
  }

  if (compute_jac)
  {
    ke += _local_ke;

    if (_has_diag_save_in && jvar == _var.number())
    {
      unsigned int rows = ke.m();
      DenseVector<Number> diag(rows);
      for (unsigned int i = 0; i < rows; i++)
        diag(i) = _local_ke(i,i);

      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i = 0; i < _diag_save_in.size(); i++)
        _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
    }
  }

  /*
  /// Loop over all the phases
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    Real mobility;
    Real dmobility;
    /// Define variables used to ensure mass conservation
    Real total_mass_out = 0;
    Real total_in = 0;

    /// The following holds derivatives of these
    std::vector<Real> dtotal_mass_out;
    std::vector<Real> dtotal_in;
    if (compute_jac)
    {
      dtotal_mass_out.resize(num_nodes);
      dtotal_in.resize(num_nodes);

      for (unsigned int n = 0; n < num_nodes ; ++n)
      {
        dtotal_mass_out[n] = 0;
        dtotal_in[n] = 0;
      }
    }

    /// Perform the upwinding using the mobility (relative permeability * fluid density / fluid viscosity)
    for (unsigned int n = 0; n < num_nodes ; ++n)
      {
        if (_local_re(n) >= 0 || reached_steady) // upstream node
        {
          /// The mobility at the upstream node
          // TODO: somewhere make sure that viscosity mustn't be zero - maybe in the viscosity material?
          mobility = _relative_permeability[n][ph] * _fluid_density_node[n][ph] / _fluid_viscosity[n][ph];

          if (compute_jac)
          {
            /// The derivative of the mobility wrt the PorousFlow variable
            dmobility = _fluid_density_node[n][ph] * _drelative_permeability_dvar[n][ph][pvar] / _fluid_viscosity[n][ph];
            dmobility += _relative_permeability[n][ph] * _dfluid_density_node_dvar[n][ph][pvar] / _fluid_viscosity[n][ph];
            dmobility -= mobility * _dfluid_viscosity_dvar[n][ph][pvar];

            for (_j = 0; _j < _phi.size(); _j++)
              _local_ke(n, _j) *= mobility;

            _local_ke(n, n) += dmobility * _local_re(n);

            for (_j = 0; _j < _phi.size(); _j++)
              dtotal_mass_out[_j] += _local_ke(n, _j);
          }
          _local_re(n) *= mobility;
          total_mass_out += _local_re(n);
        }
        else
        {
          total_in -= _local_re(n); /// note the -= means the result is positive
          if (compute_jac)
            for (_j = 0; _j < _phi.size(); _j++)
              dtotal_in[_j] -= _local_ke(n, _j);
        }
      }

    /// Conserve mass over all phases by proportioning the total_mass_out mass to the inflow nodes, weighted by their _local_re values
    if (!reached_steady)
    {
      for (unsigned int n = 0; n < num_nodes; ++n)
      {
        if (_local_re(n) < 0)
        {
          if (compute_jac)
            for (_j = 0; _j < _phi.size(); _j++)
            {
              _local_ke(n, _j) *= total_mass_out / total_in;
              _local_ke(n, _j) += _local_re(n) * (dtotal_mass_out[_j] / total_in - dtotal_in[_j] * total_mass_out / total_in / total_in);
            }
          _local_re(n) *= total_mass_out / total_in;
        }
      }
    }
  }

  /// Add results to the Residual or Jacobian
  if (compute_res)
  {
    re += _local_re;

    if (_has_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i = 0; i < _save_in.size(); i++)
        _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
    }
  }

  if (compute_jac)
  {
    ke += _local_ke;

    if (_has_diag_save_in && jvar == _var.number())
    {
      unsigned int rows = ke.m();
      DenseVector<Number> diag(rows);
      for (unsigned int i = 0; i < rows; i++)
        diag(i) = _local_ke(i,i);

      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i = 0; i < _diag_save_in.size(); i++)
        _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
    }
  }
  */
}
