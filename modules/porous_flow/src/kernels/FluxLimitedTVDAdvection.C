//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluxLimitedTVDAdvection.h"

registerMooseObject("PorousFlowApp", FluxLimitedTVDAdvection);

template <>
InputParameters
validParams<FluxLimitedTVDAdvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Conservative form of $\\nabla \\cdot \\vec{v} u$ (advection), using "
                             "the Flux Limited TVD scheme invented by Kuzmin and Turek");
  params.addRequiredParam<UserObjectName>("advective_flux_calculator",
                                          "AdvectiveFluxCalculator UserObject");
  return params;
}

FluxLimitedTVDAdvection::FluxLimitedTVDAdvection(const InputParameters & parameters)
  : Kernel(parameters),
    _u_nodal(_var.dofValues()),
    _fluo(getUserObject<AdvectiveFluxCalculator>("advective_flux_calculator"))
{
}

Real
FluxLimitedTVDAdvection::computeQpResidual()
{
  mooseError("FluxLimitedTVDAdvection::computeQpResidual() called\n");
  return 0.0;
}

void
FluxLimitedTVDAdvection::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());
  precalculateResidual();

  tvd();

  _local_re = _flux_out;
  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

void
FluxLimitedTVDAdvection::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());
  precalculateJacobian();

  tvd();

  // TODO: _dflux_out_dvar contains all the derivatives wrt to nodes not in this element.  Include
  // those too!
  for (unsigned i = 0; i < _current_elem->n_nodes(); ++i)
    for (unsigned j = 0; j < _current_elem->n_nodes(); ++j)
    {
      const dof_id_type node_id_j = _current_elem->node_id(j);
      _local_ke(i, j) = _dflux_out_dvar[i][node_id_j];
    }
  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
      var->sys().solution().add_vector(diag, var->dofIndices());
  }
}

void
FluxLimitedTVDAdvection::tvd()
{
  // The number of nodes in the element
  const unsigned int num_nodes = _test.size();

  _flux_out.resize(num_nodes);
  _flux_out.zero();

  // Retrieve KuzminTurek K matrix from the AdvectiveFluxCalculator
  // See Eqns (18)-(20)
  std::vector<std::vector<Real>> kk(num_nodes);
  for (unsigned i = 0; i < num_nodes; ++i)
  {
    kk[i].assign(num_nodes, 0.0);
    for (unsigned j = 0; j < num_nodes; ++j)
      kk[i][j] = _fluo.getKij(_current_elem->node_id(i), _current_elem->node_id(j)) /
                 _fluo.getValence(_current_elem->node_id(i), _current_elem->node_id(j));
  }

  // Calculate KuzminTurek D matrix
  // See Eqn (32)
  // This adds artificial diffusion, which eliminates any spurious oscillations
  // The idea is that D will remove all negative off-diagonal elements when it is added to K
  // This is identical to full upwinding
  std::vector<std::vector<Real>> dd(num_nodes);
  for (unsigned i = 0; i < num_nodes; ++i)
  {
    dd[i].assign(num_nodes, 0.0);
    for (unsigned j = 0; j < num_nodes; ++j)
    {
      if (i == j)
        continue;
      dd[i][j] = std::max(0.0, std::max(-kk[i][j], -kk[j][i]));
      dd[i][i] -= dd[i][j];
    }
  }

  // Calculate KuzminTurek L matrix
  // See Fig 2: L = K + D
  std::vector<std::vector<Real>> ll(num_nodes, std::vector<Real>(num_nodes));
  for (unsigned i = 0; i < num_nodes; ++i)
    for (unsigned j = 0; j < num_nodes; ++j)
      ll[i][j] = kk[i][j] + dd[i][j];

  // Retrieve KuzminTurek R matrices
  // See Eqns (49) and (12)
  std::vector<Real> rPlus(num_nodes);
  std::vector<Real> rMinus(num_nodes);
  std::vector<std::map<dof_id_type, Real>> drPlus(num_nodes); // drPlus[i][j] = d(rPlus[i])/du[j]
  std::vector<std::map<dof_id_type, Real>> drMinus(num_nodes);
  for (unsigned i = 0; i < num_nodes; ++i)
  {
    rPlus[i] = _fluo.rPlus(_current_elem->node_id(i), drPlus[i]);
    rMinus[i] = _fluo.rMinus(_current_elem->node_id(i), drMinus[i]);
  }

  // Calculate KuzminTurek f^{a} matrix
  // This is the antidiffusive flux
  // See Eqn (50)
  std::vector<std::vector<Real>> fa(num_nodes);
  std::vector<std::vector<std::map<dof_id_type, Real>>> dfa(
      num_nodes); // dfa[i][j][global_id] = d(fa[i][j])/du[global_id]
  for (unsigned i = 0; i < num_nodes; ++i)
  {
    fa[i].resize(num_nodes, 0.0);
    dfa[i].resize(num_nodes);
    for (unsigned j = 0; j < num_nodes; ++j)
    {
      // use drPlus to determine all the nodes connected with node i
      for (const auto & dof_deriv : drPlus[i])
        dfa[i][j][dof_deriv.first] = 0.0;
    }
  }
  for (unsigned i = 0; i < num_nodes; ++i)
  {
    const dof_id_type node_id_i = _current_elem->node_id(i);
    for (unsigned j = 0; j < num_nodes; ++j)
    {
      if (j == i)
        continue;
      const dof_id_type node_id_j = _current_elem->node_id(j);
      if (ll[j][i] >= ll[i][j]) // i is upwind of j.
      {
        Real prefactor = 0.0;
        std::map<dof_id_type, Real>
            dprefactor_du; // dprefactor_dr[global_id] = d(prefactor)/du[global_id];
        for (const auto & dof_deriv : drPlus[i])
          dprefactor_du[dof_deriv.first] = 0.0;
        if (_u_nodal[i] >= _u_nodal[j])
        {
          if (ll[j][i] <= rPlus[i] * dd[i][j])
            prefactor = ll[j][i];
          else
          {
            prefactor = rPlus[i] * dd[i][j];
            for (const auto & dof_deriv : drPlus[i])
              dprefactor_du[dof_deriv.first] = dof_deriv.second * dd[i][j];
          }
        }
        else
        {
          if (ll[j][i] <= rMinus[i] * dd[i][j])
            prefactor = ll[j][i];
          else
          {
            prefactor = rMinus[i] * dd[i][j];
            for (const auto & dof_deriv : drMinus[i])
              dprefactor_du[dof_deriv.first] = dof_deriv.second * dd[i][j];
          }
        }
        fa[i][j] = prefactor * (_u_nodal[i] - _u_nodal[j]);
        dfa[i][j][node_id_i] = prefactor;
        dfa[i][j][node_id_j] = -prefactor;
        for (const auto & dof_deriv : dprefactor_du)
          dfa[i][j][dof_deriv.first] += dof_deriv.second * (_u_nodal[i] - _u_nodal[j]);
      }
    }
  }
  for (unsigned i = 0; i < num_nodes; ++i)
    for (unsigned j = 0; j < num_nodes; ++j)
    {
      if (j == i)
        continue;
      if (ll[j][i] < ll[i][j]) // i is downwind of j
      {
        fa[i][j] = -fa[j][i];
        for (const auto & dof_deriv : dfa[j][i])
          dfa[i][j][dof_deriv.first] = -dof_deriv.second;
      }
    }

  // Size _dflux_out_dvar appropriately
  _dflux_out_dvar.resize(num_nodes);
  // use drPlus to determine all the nodes connected with node i
  for (unsigned i = 0; i < num_nodes; ++i)
    for (const auto & dof_deriv : drPlus[i])
      _dflux_out_dvar[i][dof_deriv.first] = 0.0;

  // Add everything together
  // See step 3 in Fig 2, noting Eqn (36)
  for (unsigned i = 0; i < num_nodes; ++i)
    for (unsigned j = 0; j < num_nodes; ++j)
    {
      // negative sign because residual = -Lu (KT equation (19))
      _flux_out(i) -= ll[i][j] * _u_nodal[j] + fa[i][j];

      const dof_id_type node_id_j = _current_elem->node_id(j);
      _dflux_out_dvar[i][node_id_j] -= ll[i][j];
      for (const auto & dof_deriv : dfa[i][j])
        _dflux_out_dvar[i][dof_deriv.first] -= dof_deriv.second;
    }
}
