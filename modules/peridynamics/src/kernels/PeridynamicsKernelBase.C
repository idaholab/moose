//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeridynamicsKernelBase.h"
#include "PeridynamicsMesh.h"

InputParameters
PeridynamicsKernelBase::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Base class for calculating the residual and Jacobian for the peridynamic kernels");

  params.addParam<bool>("full_jacobian",
                        false,
                        "Whether to include coupling terms with nodes connected to neighbors in "
                        "the Jacobian matrix for state based formulations");

  return params;
}

PeridynamicsKernelBase::PeridynamicsKernelBase(const InputParameters & parameters)
  : Kernel(parameters),
    _bond_status_var(&_subproblem.getStandardVariable(_tid, "bond_status")),
    _use_full_jacobian(getParam<bool>("full_jacobian")),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _dim(_pdmesh.dimension()),
    _nnodes(2),
    _node_vol(_nnodes),
    _dg_vol_frac(_nnodes),
    _horizon_radius(_nnodes),
    _horizon_vol(_nnodes)
{
}

void
PeridynamicsKernelBase::prepare()
{
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    _node_vol[nd] = _pdmesh.getNodeVolume(_current_elem->node_id(nd));
    dof_id_type id_ji_in_ij =
        _pdmesh.getNeighborIndex(_current_elem->node_id(nd), _current_elem->node_id(1 - nd));
    _dg_vol_frac[nd] =
        _pdmesh.getHorizonSubsetVolumeFraction(_current_elem->node_id(nd), id_ji_in_ij);
    _horizon_radius[nd] = _pdmesh.getHorizon(_current_elem->node_id(nd));
    _horizon_vol[nd] = _pdmesh.getHorizonVolume(_current_elem->node_id(nd));
  }

  _origin_vec = _pdmesh.getNodeCoord(_current_elem->node_id(1)) -
                _pdmesh.getNodeCoord(_current_elem->node_id(0));
  _bond_status = _bond_status_var->getElementalValue(_current_elem);
}

void
PeridynamicsKernelBase::computeResidual()
{
  prepare();

  prepareVectorTag(_assembly, _var.number());
  mooseAssert(_local_re.size() == _nnodes,
              "PeridynamicsKernelBase is designed to only work with EDGE2 elements");
  computeLocalResidual();
  accumulateTaggedLocalResidual();

  if (_has_save_in)
    for (unsigned int i = 0; i < _save_in.size(); ++i)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());

  _local_re.zero();

  computeNonlocalResidual();
}

void
PeridynamicsKernelBase::computeJacobian()
{
  prepare();

  prepareMatrixTag(_assembly, _var.number(), _var.number());
  computeLocalJacobian();
  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; ++i)
      diag(i) = _local_ke(i, i);

    for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }

  _local_ke.zero();

  if (_use_full_jacobian)
    computeNonlocalJacobian();
}
