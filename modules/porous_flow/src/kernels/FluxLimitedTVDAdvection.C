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
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity vector");
  params.addRequiredParam<UserObjectName>("flux_limiter_uo", "FluxLimiter UO");
  return params;
}

FluxLimitedTVDAdvection::FluxLimitedTVDAdvection(const InputParameters & parameters)
  : Kernel(parameters),
    _velocity(getParam<RealVectorValue>("velocity")),
    _u_nodal(_var.dofValues()),
    _fluo(getUserObject<FluxLimiter>("flux_limiter_uo"))
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

  //_fluo.val_at_node(*_current_elem->node_ptr(0));
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
FluxLimitedTVDAdvection::tvd()
{
  // The number of nodes in the element
  const unsigned int num_nodes = _test.size();

  _flux_out.resize(num_nodes);
  _flux_out.zero();

  // Calculate KuzminTurek K matrix
  // See Eqns (18)-(20)
  std::vector<std::vector<Real>> kk(num_nodes);
  for (unsigned i = 0; i < num_nodes; ++i)
  {
    kk[i].assign(num_nodes, 0.0);
    for (unsigned j = 0; j < num_nodes; ++j)
      for (unsigned qp = 0; qp < _qrule->n_points(); ++qp)
        // there is no negative here because i've used integration by parts to put the grad onto
        // _test
        kk[i][j] += _JxW[qp] * _coord[qp] * (_grad_test[i][qp] * _velocity) * _test[j][qp];
  }

  // ---------------------------------------------------------------------------
  // |                               NOTE !!                                   |
  // | If we use the K calculated above, we'll be computed P and Q based on    |
  // | the fluxes WITHIN THIS ELEMENT ONLY                                     |
  // | That is incorrect: instead the P and Q should be computed by looking at |
  // | the fluxes from a node to ALL its joining nodes, not just the ones      |
  // | in this element.                                                        |
  // | Hence, i just use the K hard-coded into FluxLimiter:                    |
  // ---------------------------------------------------------------------------
  for (unsigned i = 0; i < num_nodes; ++i)
    for (unsigned j = 0; j < num_nodes; ++j)
      kk[i][j] =
          _fluo.getKij(*_current_elem->node_ptr(i), *_current_elem->node_ptr(j)) * _velocity(0);

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

  // Calculate KuzminTurek R matrices
  // See Eqns (49) and (12)
  std::vector<Real> rPlus(num_nodes);
  std::vector<Real> rMinus(num_nodes);
  for (unsigned i = 0; i < num_nodes; ++i)
  {
    rPlus[i] = _fluo.rPlus(*_current_elem->node_ptr(i));
    rMinus[i] = _fluo.rMinus(*_current_elem->node_ptr(i));
  }

  // Calculate KuzminTurek f^{a} matrix
  // This is the antidiffusive flux
  // See Eqn (50)
  std::vector<std::vector<Real>> fa(num_nodes);
  for (unsigned i = 0; i < num_nodes; ++i)
    fa[i].resize(num_nodes, 0.0);
  for (unsigned i = 0; i < num_nodes; ++i)
    for (unsigned j = 0; j < num_nodes; ++j)
    {
      if (j == i)
        continue;
      if (ll[j][i] >= ll[i][j]) // i is upwind of j.
      {
        if (_u_nodal[i] >= _u_nodal[j])
          fa[i][j] = std::min(rPlus[i] * dd[i][j], ll[j][i]) * (_u_nodal[i] - _u_nodal[j]);
        else
          fa[i][j] = std::min(rMinus[i] * dd[i][j], ll[j][i]) * (_u_nodal[i] - _u_nodal[j]);
      }
    }
  for (unsigned i = 0; i < num_nodes; ++i)
    for (unsigned j = 0; j < num_nodes; ++j)
    {
      if (j == i)
        continue;
      if (ll[j][i] < ll[i][j]) // i is downwind of j
        fa[i][j] = -fa[j][i];
    }

  // Add everything together
  // See step 3 in Fig 2, noting Eqn (36)
  for (unsigned i = 0; i < num_nodes; ++i)
    for (unsigned j = 0; j < num_nodes; ++j)
      // negative sign because residual = -Lu (KT equation (19))
      _flux_out(i) -= ll[i][j] * _u_nodal[j] + fa[i][j];
  return;
}
