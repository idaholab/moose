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
  : Kernel(parameters), _fluo(getUserObject<AdvectiveFluxCalculator>("advective_flux_calculator"))
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

  // get the residual contributions from _fluo
  for (unsigned i = 0; i < _current_elem->n_nodes(); ++i)
  {
    const dof_id_type node_id_i = _current_elem->node_id(i);
    _local_re(i) = _fluo.getFluxOut(node_id_i) / _fluo.getValence(node_id_i, node_id_i);
  }

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

  // get the Jacobian contributions from _fluo
  for (unsigned i = 0; i < _current_elem->n_nodes(); ++i)
  {
    const dof_id_type node_id_i = _current_elem->node_id(i);
    for (unsigned j = 0; j < _current_elem->n_nodes(); ++j)
    {
      const dof_id_type node_id_j = _current_elem->node_id(j);
      _local_ke(i, j) =
          _fluo.getdFluxOutdu(node_id_i, node_id_j) / _fluo.getValence(node_id_i, node_id_j);
    }
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
