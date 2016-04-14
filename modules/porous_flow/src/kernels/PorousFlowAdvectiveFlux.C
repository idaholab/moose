/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowAdvectiveFlux.h"
#include "Assembly.h"
#include "libmesh/quadrature.h"


template<>
InputParameters validParams<PorousFlowAdvectiveFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<unsigned int>("component_index", 0, "The index corresponding to the component for this kernel");
  params.addRequiredParam<RealVectorValue>("gravity", "Gravitational acceleration vector downwards (m/s^2)");
  params.addRequiredParam<UserObjectName>("PorousFlowVarNamesUO", "The UserObject that holds the list of PorousFlow variable names");
  params.addClassDescription("Fully-upwinded advective flux of the component given by component_index");
  return params;
}

PorousFlowAdvectiveFlux::PorousFlowAdvectiveFlux(const InputParameters & parameters) :
  Kernel(parameters),
  _permeability(getMaterialProperty<RealTensorValue>("permeability")),
  _gravity(getParam<RealVectorValue>("gravity")),
  _fluid_density_node(getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density")),
  _dfluid_density_node_dvar(getMaterialProperty<std::vector<Real> >("dPorousFlow_fluid_phase_density_dvar")),
  _fluid_density_qp(getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density_qp")),
  _dfluid_density_qp_dvar(getMaterialProperty<std::vector<Real> >("dPorousFlow_fluid_phase_density_qp_dvar")),
  _fluid_viscosity(getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_viscosity")),
  _mass_fractions(getMaterialProperty<std::vector<std::vector<Real> > >("PorousFlow_mass_frac")),
  _dmass_fractions_dvar(getMaterialProperty<std::vector<std::vector<std::vector<Real> > > >("dPorousFlow_mass_frac_dvar")),
  _grad_p(getMaterialProperty<std::vector<RealGradient> >("PorousFlow_grad_porepressure")),
  _dgrad_p_dgrad_var(getMaterialProperty<std::vector<std::vector<Real> >("dPorousFlow_grad_porepressure_dgradvar")),
  _relative_permeability(getMaterialProperty<std::vector<Real> >("PorousFlow_relative_permeabilty")),
  _porousflow_varname_UO(getUserObject<PorousFlowVarNames>("PorousFlowVarNamesUO")),
  _component_index(getParam<unsigned int>("component_index"))

{
  /// Make sure that this kernels variable is a valid PorousFlow variable
  if (_porousflow_varname_UO.not_porousflow_var(_var.number()))
    mooseError("Variable " << _var.name() << " in the " << _name << " kernel is not a valid PorousFlow variable");

  _num_phases = _porousflow_varname_UO.numberPhases();
}

Real PorousFlowAdvectiveFlux::computeQpResidual()
{
  // TODO: want these densities to be the qp densities
  // TODO: check that these grad_p's at the qp?
  RealVectorValue qpresidual = 0.0;

  for (unsigned ph = 0; ph < _num_phases; ++ph)
    qpresidual += _mass_fractions[_qp][ph][_component_index] * (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity);

  return _grad_test[_i][_qp] * (_permeability[_qp] * qpresidual);
}

void PorousFlowAdvectiveFlux::computeResidual()
{
  upwind(true, false, 0.);
}


Real PorousFlow::computeQpJac(unsigned int pvar)
{
  /// If the variable is not a valid PorousFlow variable, return 0
  if (_porousflow_varname_UO.not_porousflow_var(pvar))
    return 0.0;

  RealVectorValue qpjacobian = 0.0;

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    qpjacobian += _phi[_j][_qp] * _dmass_fractions_dvar[_qp][ph][_component_index][pvar] * (_grad_p[_qp][ph] - _fluid_density_qp[_qp][ph] * _gravity);
    qpjacobian += _mass_fractions[_qp][ph][_component_index] * (_grad_phi[_j][_qp] * _dgrad_p_dgrad_var[_qp][ph] - _phi[_j][_qp] * _dfluid_density_qp_dvar[_qp][ph][pvar] * _gravity);
  }

  return _grad_test[_i][_qp] * (_permeability[_qp] * qpjacobian);
}

Real PorousFlowAdvectiveFlux::computeQpJacobian()
{
  return computeQpJac(_porousflow_varname_UO.porflow_var_num(_var.number()));;
}

void PorousFlowAdvectiveFlux::computeJacobian()
{
   upwind(false, true, _var.number());
}

Real PorousFlowAdvectiveFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  return computeQpJac(_porousflow_varname_UO.porflow_var_num(jvar));;
}

void PorousFlowAdvectiveFlux::computeOffDiagJacobian(unsigned int jvar)
{
   upwind(false, true, jvar);
}


void PorousFlowAdvectiveFlux::upwind(bool compute_res, bool compute_jac, unsigned int jvar)
{
  if (compute_jac && _porousflow_varname_UO.not_porousflow_var(jvar))
    return;

  /// We require the mobility calculated at the nodes of the element. Additionally,
  /// in order to call the derivatives for the Jacobian from the FluidState UserObject,
  /// determine the node id's of the nodes in this element

  unsigned int num_nodes = _test.size();
  std::vector<Real> mobility;
  mobility.resize(num_nodes);

  std::vector<unsigned int> elem_node_ids;
  elem_node_ids.resize(num_nodes);

  for (unsigned int n = 0; n < num_nodes; ++n)
  {
    elem_node_ids[n] = _current_elem->get_node(n)->id();
    mobility[n] = _fluid_state.getNodalProperty("mobility", elem_node_ids[n], _phase_index);
  }

  /// Compute the residual and jacobian without the mobility terms. Even if we are computing the jacobian
  /// we still need this in order to see which nodes are upwind and which are downwind.
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);

  /// Form the _local_re contribution to the residual without the mobility term
 for (_i = 0; _i < _test.size(); _i++)
   for (_qp = 0; _qp < _qrule->n_points(); _qp++)
     _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();

 if (compute_jac)
 {
    _local_ke.resize(ke.m(), ke.n());
    _local_ke.zero();

    if (jvar == _var.number())
    {
      for (_i = 0; _i < _test.size(); _i++)
        for (_j = 0; _j < _phi.size(); _j++)
          for (_qp = 0; _qp < _qrule->n_points(); _qp++)
            _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();
    }
    else
    {
      for (_i = 0; _i < _test.size(); _i++)
        for (_j = 0; _j < _phi.size(); _j++)
          for (_qp = 0; _qp < _qrule->n_points(); _qp++)
            _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
    }
  }

  // Now perform the upwinding by multiplying the residuals at the
  // upstream nodes by their mobilities
  //
  // The residual for the kernel is the darcy flux.
  // This is
  // R_i = int{mobility*flux_no_mob} = int{mobility*grad(pot)*permeability*grad(test_i)}
  // for node i.  where int is the integral over the element.
  // However, in fully-upwind, the first step is to take the mobility outside the
  // integral, which was done in the _local_re calculation above.
  //
  // NOTE: Physically _local_re(_i) is a measure of fluid flowing out of node i
  // If we had left in mobility, it would be exactly the mass flux flowing out of node i.
  //
  // This leads to the definition of upwinding:
  // ***
  // If _local_re(i) is positive then we use mobility_i.  That is
  // we use the upwind value of mobility.
  // ***
  //
  // The final subtle thing is we must also conserve fluid mass: the total mass
  // flowing out of node i must be the sum of the masses flowing
  // into the other nodes.

  // FIRST:
  // this is a dirty way of getting around precision loss problems
  // and problems at steadystate where upwinding oscillates from
  // node-to-node causing nonconvergence.
  bool reached_steady = true;
  for (unsigned int n = 0; n < num_nodes ; ++n)
  {
    if (_local_re(n) >= 1E-20)
    {
      reached_steady = false;
      break;
    }
  }

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

  /// Perform the upwinding
    for (unsigned int n = 0; n < num_nodes ; ++n)
    {
      if (_local_re(n) >= 0 || reached_steady) // upstream node
      {
        if (compute_jac)
        {
          Real dmobility = 0.;

          if (jvar_type == "pressure")
            dmobility = _fluid_state.getNodalProperty("dmobility_dp", elem_node_ids[n], _phase_index);

          else if (jvar_type == "saturation")
            dmobility = _fluid_state.getNodalProperty("dmobility_ds", elem_node_ids[n], _phase_index);

          else if (jvar_type == "mass_fraction")
            dmobility = _fluid_state.getNodalProperty("dmobility_dx", elem_node_ids[n], _phase_index, _component_index);

          for (_j = 0; _j < _phi.size(); _j++)
            _local_ke(n, _j) *= mobility[n];

          _local_ke(n, n) += dmobility * _local_re(n);

          for (_j = 0; _j < _phi.size(); _j++)
            dtotal_mass_out[_j] += _local_ke(n, _j);
        }
        _local_re(n) *= mobility[n];
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
}
