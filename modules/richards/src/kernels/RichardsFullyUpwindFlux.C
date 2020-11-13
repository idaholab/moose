//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsFullyUpwindFlux.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariable.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

registerMooseObject("RichardsApp", RichardsFullyUpwindFlux);

InputParameters
RichardsFullyUpwindFlux::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<std::vector<UserObjectName>>(
      "relperm_UO", "List of names of user objects that define relative permeability");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "seff_UO",
      "List of name of user objects that define effective saturation as a function of "
      "pressure list");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "density_UO", "List of names of user objects that define the fluid density");
  params.addRequiredParam<UserObjectName>(
      "richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  return params;
}

RichardsFullyUpwindFlux::RichardsFullyUpwindFlux(const InputParameters & parameters)
  : Kernel(parameters),
    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _num_p(_richards_name_UO.num_v()),
    _pvar(_richards_name_UO.richards_var_num(_var.number())),
    _density_UO(getUserObjectByName<RichardsDensity>(
        getParam<std::vector<UserObjectName>>("density_UO")[_pvar])),
    _seff_UO(
        getUserObjectByName<RichardsSeff>(getParam<std::vector<UserObjectName>>("seff_UO")[_pvar])),
    _relperm_UO(getUserObjectByName<RichardsRelPerm>(
        getParam<std::vector<UserObjectName>>("relperm_UO")[_pvar])),
    _viscosity(getMaterialProperty<std::vector<Real>>("viscosity")),
    _flux_no_mob(getMaterialProperty<std::vector<RealVectorValue>>("flux_no_mob")),
    _dflux_no_mob_dv(
        getMaterialProperty<std::vector<std::vector<RealVectorValue>>>("dflux_no_mob_dv")),
    _dflux_no_mob_dgradv(
        getMaterialProperty<std::vector<std::vector<RealTensorValue>>>("dflux_no_mob_dgradv")),
    _num_nodes(0),
    _mobility(0),
    _dmobility_dv(0)
{
  _ps_at_nodes.resize(_num_p);
  for (unsigned int pnum = 0; pnum < _num_p; ++pnum)
    _ps_at_nodes[pnum] = _richards_name_UO.nodal_var(pnum);
}

void
RichardsFullyUpwindFlux::prepareNodalValues()
{
  _num_nodes = (*_ps_at_nodes[_pvar]).size();

  Real p;
  Real density;
  Real ddensity_dp;
  Real seff;
  std::vector<Real> dseff_dp;
  Real relperm;
  Real drelperm_ds;
  _mobility.resize(_num_nodes);
  _dmobility_dv.resize(_num_nodes);
  dseff_dp.resize(_num_p);
  for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
  {
    // retrieve and calculate basic things at the node
    p = (*_ps_at_nodes[_pvar])[nodenum];   // pressure of fluid _pvar at node nodenum
    density = _density_UO.density(p);      // density of fluid _pvar at node nodenum
    ddensity_dp = _density_UO.ddensity(p); // d(density)/dP
    seff =
        _seff_UO.seff(_ps_at_nodes, nodenum); // effective saturation of fluid _pvar at node nodenum
    _seff_UO.dseff(_ps_at_nodes, nodenum, dseff_dp); // d(seff)/d(P_ph), for ph = 0, ..., _num_p - 1
    relperm = _relperm_UO.relperm(seff); // relative permeability of fluid _pvar at node nodenum
    drelperm_ds = _relperm_UO.drelperm(seff); // d(relperm)/dseff

    // calculate the mobility and its derivatives wrt (variable_ph = porepressure_ph)
    _mobility[nodenum] =
        density * relperm / _viscosity[0][_pvar]; // assume viscosity is constant throughout element
    _dmobility_dv[nodenum].resize(_num_p);
    for (unsigned int ph = 0; ph < _num_p; ++ph)
      _dmobility_dv[nodenum][ph] = density * drelperm_ds * dseff_dp[ph] / _viscosity[0][_pvar];
    _dmobility_dv[nodenum][_pvar] += ddensity_dp * relperm / _viscosity[0][_pvar];
  }
}

Real
RichardsFullyUpwindFlux::computeQpResidual()
{
  // note this is not the complete residual:
  // the upwind mobility parts get added in computeResidual
  return _grad_test[_i][_qp] * _flux_no_mob[_qp][_pvar];
}

void
RichardsFullyUpwindFlux::computeResidual()
{
  upwind(true, false, 0);
  return;
}

void
RichardsFullyUpwindFlux::computeOffDiagJacobian(const unsigned int jvar)
{
  upwind(false, true, jvar);
  return;
}

Real
RichardsFullyUpwindFlux::computeQpJac(unsigned int dvar)
{
  // this is just the derivative of the flux WITHOUT the upstream mobility terms
  // Those terms get added in during computeJacobian()
  return _grad_test[_i][_qp] * (_dflux_no_mob_dgradv[_qp][_pvar][dvar] * _grad_phi[_j][_qp] +
                                _dflux_no_mob_dv[_qp][_pvar][dvar] * _phi[_j][_qp]);
}

void
RichardsFullyUpwindFlux::upwind(bool compute_res, bool compute_jac, unsigned int jvar)
{
  if (compute_jac && _richards_name_UO.not_richards_var(jvar))
    return;

  // calculate the mobility values and their derivatives
  prepareNodalValues();

  // compute the residual without the mobility terms
  // Even if we are computing the jacobian we still need this
  // in order to see which nodes are upwind and which are downwind
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  _local_re.resize(re.size());
  _local_re.zero();

  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();

  const unsigned int dvar = _richards_name_UO.richards_var_num(jvar);
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);
  if (compute_jac)
  {
    _local_ke.resize(ke.m(), ke.n());
    _local_ke.zero();

    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJac(dvar);
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
  // I'm not sure if i actually need to do this in moose.  Certainly
  // in cosflow it is necessary.
  // I will code a better algorithm if necessary
  bool reached_steady = true;
  for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
  {
    if (_local_re(nodenum) >= 1E-20)
    {
      reached_steady = false;
      break;
    }
  }

  // DEFINE VARIABLES USED TO ENSURE MASS CONSERVATION
  // total mass out - used for mass conservation
  Real total_mass_out = 0;
  // total flux in
  Real total_in = 0;

  // the following holds derivatives of these
  std::vector<Real> dtotal_mass_out;
  std::vector<Real> dtotal_in;
  if (compute_jac)
  {
    dtotal_mass_out.resize(_num_nodes);
    dtotal_in.resize(_num_nodes);
    for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
    {
      dtotal_mass_out[nodenum] = 0;
      dtotal_in[nodenum] = 0;
    }
  }

  // PERFORM THE UPWINDING!
  for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
  {
    if (_local_re(nodenum) >= 0 || reached_steady) // upstream node
    {
      if (compute_jac)
      {
        for (_j = 0; _j < _phi.size(); _j++)
          _local_ke(nodenum, _j) *= _mobility[nodenum];
        _local_ke(nodenum, nodenum) += _dmobility_dv[nodenum][dvar] * _local_re(nodenum);
        for (_j = 0; _j < _phi.size(); _j++)
          dtotal_mass_out[_j] += _local_ke(nodenum, _j);
      }
      _local_re(nodenum) *= _mobility[nodenum];
      total_mass_out += _local_re(nodenum);
    }
    else
    {
      total_in -= _local_re(nodenum); // note the -= means the result is positive
      if (compute_jac)
        for (_j = 0; _j < _phi.size(); _j++)
          dtotal_in[_j] -= _local_ke(nodenum, _j);
    }
  }

  // CONSERVE MASS
  // proportion the total_mass_out mass to the inflow nodes, weighting by their _local_re values
  if (!reached_steady)
    for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
      if (_local_re(nodenum) < 0)
      {
        if (compute_jac)
          for (_j = 0; _j < _phi.size(); _j++)
          {
            _local_ke(nodenum, _j) *= total_mass_out / total_in;
            _local_ke(nodenum, _j) +=
                _local_re(nodenum) * (dtotal_mass_out[_j] / total_in -
                                      dtotal_in[_j] * total_mass_out / total_in / total_in);
          }
        _local_re(nodenum) *= total_mass_out / total_in;
      }

  // ADD RESULTS TO RESIDUAL OR JACOBIAN
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

    if (_has_diag_save_in && dvar == _pvar)
    {
      const unsigned int rows = ke.m();
      DenseVector<Number> diag(rows);
      for (unsigned int i = 0; i < rows; i++)
        diag(i) = _local_ke(i, i);

      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i = 0; i < _diag_save_in.size(); i++)
        _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
    }
  }
}
