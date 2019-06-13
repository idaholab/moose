//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KernelBasePD.h"
#include "PeridynamicsMesh.h"

template <>
InputParameters
validParams<KernelBasePD>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription(
      "Base class for calculating residual and Jacobian for peridynamic kernels");

  params.addParam<bool>("full_jacobian",
                        false,
                        "Whether to include coupling terms with nodes connected to neighbors in "
                        "the Jacobian matrix for state based formulations");

  return params;
}

KernelBasePD::KernelBasePD(const InputParameters & parameters)
  : Kernel(parameters),
    _bond_status_var(_subproblem.getVariable(_tid, "bond_status")),
    _use_full_jacobian(getParam<bool>("full_jacobian")),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _dim(_pdmesh.dimension()),
    _nnodes(2) // This is designed specifically to work with EDGE2 elements, which have two nodes
{
}

void
KernelBasePD::prepare()
{
  _vols_ij.resize(_nnodes);
  _dg_bond_vsum_ij.resize(_nnodes);
  _dg_node_vsum_ij.resize(_nnodes);
  _horizons_ij.resize(_nnodes);

  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    _vols_ij[nd] = _pdmesh.getVolume(_current_elem->node_id(nd));
    unsigned int id_ji_in_ij =
        _pdmesh.getNeighborID(_current_elem->node_id(nd), _current_elem->node_id(1 - nd));
    _dg_bond_vsum_ij[nd] =
        _pdmesh.getBondAssocHorizonVolume(_current_elem->node_id(nd), id_ji_in_ij);
    _dg_node_vsum_ij[nd] = _pdmesh.getBondAssocHorizonVolumeSum(_current_elem->node_id(nd));
    _horizons_ij[nd] = _pdmesh.getHorizon(_current_elem->node_id(nd));
  }

  _origin_vec_ij = *_current_elem->node_ptr(1) - *_current_elem->node_ptr(0);
  _bond_status_ij = _bond_status_var.getElementalValue(_current_elem);
}

void
KernelBasePD::computeResidual()
{
  prepare();

  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  mooseAssert(re.size() == _nnodes, "KernelBasePD is designed to only work with EDGE2 elements");
  _local_re.resize(re.size());
  _local_re.zero();

  computeLocalResidual();

  re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); ++i)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }

  _local_re.zero();

  computeNonlocalResidual();
}

void
KernelBasePD::computeJacobian()
{
  prepare();

  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  computeLocalJacobian();

  ke += _local_ke;

  if (_has_diag_save_in)
  {
    unsigned int rows = ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; ++i)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }

  _local_ke.zero();

  if (_use_full_jacobian)
    computeNonlocalJacobian();
}
