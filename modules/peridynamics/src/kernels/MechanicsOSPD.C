//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicsOSPD.h"
#include "PeridynamicsMesh.h"

registerMooseObject("PeridynamicsApp", MechanicsOSPD);

template <>
InputParameters
validParams<MechanicsOSPD>()
{
  InputParameters params = validParams<MechanicsBasePD>();
  params.addClassDescription("Class for calculating residual and Jacobian for Ordinary State-based "
                             "PeriDynamic mechanics formulation");

  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the variable this kernel acts on (0 for x, 1 for y, 2 for z)");

  return params;
}

MechanicsOSPD::MechanicsOSPD(const InputParameters & parameters)
  : MechanicsBasePD(parameters),
    _bond_force_ij(getMaterialProperty<Real>("bond_force_ij")),
    _bond_force_i_j(getMaterialProperty<Real>("bond_force_i_j")),
    _bond_dfdU_ij(getMaterialProperty<Real>("bond_dfdU_ij")),
    _bond_dfdU_i_j(getMaterialProperty<Real>("bond_dfdU_i_j")),
    _bond_dfdT_ij(getMaterialProperty<Real>("bond_dfdT_ij")),
    _bond_dfdT_i_j(getMaterialProperty<Real>("bond_dfdT_i_j")),
    _component(getParam<unsigned int>("component"))
{
}

void
MechanicsOSPD::computeLocalResidual()
{
  // H term
  _local_re(0) = -_bond_force_ij[0] * _cur_ori_ij(_component) * _bond_status_ij;
  _local_re(1) = -_local_re(0);
}

void
MechanicsOSPD::computeNonlocalResidual()
{
  // P and Q terms
  for (unsigned int cur_nd = 0; cur_nd < 2; ++cur_nd)
  {
    // calculation of residual contribution to current_node's neighbors
    std::vector<dof_id_type> ivardofs(2);
    ivardofs[cur_nd] = _ivardofs_ij[cur_nd];
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(cur_nd));
    std::vector<dof_id_type> bonds = _pdmesh.getAssocBonds(_current_elem->node_id(cur_nd));
    for (unsigned int k = 0; k < neighbors.size(); ++k)
    {
      Node * node_k = _pdmesh.nodePtr(neighbors[k]);
      ivardofs[1 - cur_nd] = node_k->dof_number(_sys.number(), _var.number(), 0);
      Real vol_k = _pdmesh.getVolume(neighbors[k]);

      // obtain bond ik's origin length and current orientation
      RealGradient origin_ori_ijk = *node_k - *_pdmesh.nodePtr(_current_elem->node_id(cur_nd));

      RealGradient cur_ori_ijk(_dim);
      for (unsigned int j = 0; j < _dim; ++j)
        cur_ori_ijk(j) = origin_ori_ijk(j) + _disp_var[j]->getNodalValue(*node_k) -
                         _disp_var[j]->getNodalValue(*_current_elem->node_ptr(cur_nd));

      cur_ori_ijk /= cur_ori_ijk.norm();

      // bond status for bond k
      Real bond_status_ijk = _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[k]));

      _local_re(0) = (cur_nd == 0 ? -1 : 1) * _bond_force_i_j[cur_nd] * vol_k /
                     origin_ori_ijk.norm() * cur_ori_ijk(_component) * _bond_status_ij *
                     bond_status_ijk;
      _local_re(1) = -_local_re(0);

      // cache the residual contribution to node_i and its neighbor k using their global dof indices
      _assembly.cacheResidualNodes(_local_re, ivardofs);

      // save in the displacement residuals
      if (_has_save_in)
      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        for (unsigned int i = 0; i < _save_in.size(); ++i)
        {
          std::vector<dof_id_type> save_in_dofs(2);
          save_in_dofs[cur_nd] = _current_elem->node_ptr(cur_nd)->dof_number(
              _save_in[i]->sys().number(), _save_in[i]->number(), 0);
          save_in_dofs[1 - cur_nd] =
              node_k->dof_number(_save_in[i]->sys().number(), _save_in[i]->number(), 0);

          _save_in[i]->sys().solution().add_vector(_local_re, save_in_dofs);
        }
      }
    }
  }
}

void
MechanicsOSPD::computeLocalJacobian()
{
  Real val =
      _cur_ori_ij(_component) * _cur_ori_ij(_component) * _bond_dfdU_ij[0] +
      _bond_force_ij[0] * (1.0 - _cur_ori_ij(_component) * _cur_ori_ij(_component)) / _cur_len_ij;

  for (_i = 0; _i < _test.size(); ++_i)
    for (_j = 0; _j < _phi.size(); ++_j)
      _local_ke(_i, _j) += (_i == _j ? 1 : -1) * val * _bond_status_ij;
}

void
MechanicsOSPD::computeLocalOffDiagJacobian(unsigned int coupled_component)
{
  if (coupled_component == 3)
  {
    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < _phi.size(); ++_j)
        _local_ke(_i, _j) +=
            (_i == 1 ? 1 : -1) * _cur_ori_ij(_component) * _bond_dfdT_ij[0] * _bond_status_ij;
  }
  else
  {
    Real val =
        _cur_ori_ij(_component) * _cur_ori_ij(coupled_component) * _bond_dfdU_ij[0] -
        _bond_force_ij[0] * _cur_ori_ij(_component) * _cur_ori_ij(coupled_component) / _cur_len_ij;
    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < _phi.size(); ++_j)
        _local_ke(_i, _j) += (_i == _j ? 1 : -1) * val * _bond_status_ij;
  }
}

void
MechanicsOSPD::computeNonlocalJacobian()
{
  for (unsigned int cur_nd = 0; cur_nd < _nnodes; ++cur_nd)
  {
    // calculation of jacobian contribution to current_node's neighbors
    std::vector<dof_id_type> ivardofs(_nnodes);
    ivardofs[cur_nd] = _ivardofs_ij[cur_nd];
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(cur_nd));
    std::vector<dof_id_type> bonds = _pdmesh.getAssocBonds(_current_elem->node_id(cur_nd));
    for (unsigned int k = 0; k < neighbors.size(); ++k)
    {
      Node * node_k = _pdmesh.nodePtr(neighbors[k]);
      ivardofs[1 - cur_nd] = node_k->dof_number(_sys.number(), _var.number(), 0);
      Real vol_k = _pdmesh.getVolume(neighbors[k]);

      // obtain bond ik's origin length and current orientation
      RealGradient origin_ori_ijk = *node_k - *_pdmesh.nodePtr(_current_elem->node_id(cur_nd));

      RealGradient cur_ori_ijk(_dim);
      for (unsigned int j = 0; j < _dim; ++j)
        cur_ori_ijk(j) = origin_ori_ijk(j) + _disp_var[j]->getNodalValue(*node_k) -
                         _disp_var[j]->getNodalValue(*_current_elem->node_ptr(cur_nd));

      cur_ori_ijk /= cur_ori_ijk.norm();

      // bond status for bond k
      Real bond_status_ijk = _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[k]));

      Real val = (cur_nd == 0 ? 1 : -1) * cur_ori_ijk(_component) * _cur_ori_ij(_component) *
                 _bond_dfdU_i_j[cur_nd];

      _local_ke.zero();
      for (_i = 0; _i < _test.size(); ++_i)
        for (_j = 0; _j < _phi.size(); ++_j)
          _local_ke(_i, _j) += (_i == _j ? 1 : -1) * val / origin_ori_ijk.norm() * vol_k *
                               _bond_status_ij * bond_status_ijk;

      _assembly.cacheJacobianBlock(_local_ke, ivardofs, _ivardofs_ij, _var.scalingFactor());

      if (_has_diag_save_in)
      {
        unsigned int rows = _test.size();
        DenseVector<Real> diag(rows);
        for (unsigned int i = 0; i < rows; ++i)
          diag(i) = _local_ke(i, i);

        diag(1 - cur_nd) = 0;

        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
        {
          std::vector<dof_id_type> diag_save_in_dofs(2);
          diag_save_in_dofs[cur_nd] = _current_elem->node_ptr(cur_nd)->dof_number(
              _diag_save_in[i]->sys().number(), _diag_save_in[i]->number(), 0);
          diag_save_in_dofs[1 - cur_nd] =
              node_k->dof_number(_diag_save_in[i]->sys().number(), _diag_save_in[i]->number(), 0);

          _diag_save_in[i]->sys().solution().add_vector(diag, diag_save_in_dofs);
        }
      }

      val = _bond_force_i_j[cur_nd] * (1.0 - cur_ori_ijk(_component) * cur_ori_ijk(_component)) /
            cur_ori_ijk.norm();

      _local_ke.zero();
      for (_i = 0; _i < _test.size(); ++_i)
        for (_j = 0; _j < _phi.size(); ++_j)
          _local_ke(_i, _j) += (_i == _j ? 1 : -1) * val / origin_ori_ijk.norm() * vol_k *
                               _bond_status_ij * bond_status_ijk;

      _assembly.cacheJacobianBlock(_local_ke, ivardofs, ivardofs, _var.scalingFactor());

      if (_has_diag_save_in)
      {
        unsigned int rows = _test.size();
        DenseVector<Real> diag(rows);
        for (unsigned int i = 0; i < rows; ++i)
          diag(i) = _local_ke(i, i);

        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
        {
          std::vector<dof_id_type> diag_save_in_dofs(2);
          diag_save_in_dofs[cur_nd] = _current_elem->node_ptr(cur_nd)->dof_number(
              _diag_save_in[i]->sys().number(), _diag_save_in[i]->number(), 0);
          diag_save_in_dofs[1 - cur_nd] =
              node_k->dof_number(_diag_save_in[i]->sys().number(), _diag_save_in[i]->number(), 0);

          _diag_save_in[i]->sys().solution().add_vector(diag, diag_save_in_dofs);
        }
      }
    }
  }
}

void
MechanicsOSPD::computePDNonlocalOffDiagJacobian(unsigned int jvar_num,
                                                unsigned int coupled_component)
{
  std::vector<dof_id_type> jvardofs_ij(_nnodes);
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
    jvardofs_ij[nd] = _current_elem->node_ptr(nd)->dof_number(_sys.number(), jvar_num, 0);

  for (unsigned int cur_nd = 0; cur_nd < _nnodes; ++cur_nd)
  {
    // calculation of jacobian contribution to current_node's neighbors
    std::vector<dof_id_type> ivardofs(_nnodes), jvardofs(_nnodes);
    ivardofs[cur_nd] = _ivardofs_ij[cur_nd];
    jvardofs[cur_nd] = jvardofs_ij[cur_nd];
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(cur_nd));
    std::vector<dof_id_type> bonds = _pdmesh.getAssocBonds(_current_elem->node_id(cur_nd));
    for (unsigned int k = 0; k < neighbors.size(); ++k)
    {
      Node * node_k = _pdmesh.nodePtr(neighbors[k]);
      ivardofs[1 - cur_nd] = node_k->dof_number(_sys.number(), _var.number(), 0);
      jvardofs[1 - cur_nd] = node_k->dof_number(_sys.number(), jvar_num, 0);
      Real vol_k = _pdmesh.getVolume(neighbors[k]);

      // obtain bond k's origin length and current orientation
      RealGradient origin_ori_ijk = *node_k - *_pdmesh.nodePtr(_current_elem->node_id(cur_nd));

      RealGradient cur_ori_ijk;
      for (unsigned int j = 0; j < _dim; ++j)
        cur_ori_ijk(j) = origin_ori_ijk(j) + _disp_var[j]->getNodalValue(*node_k) -
                         _disp_var[j]->getNodalValue(*_current_elem->node_ptr(cur_nd));

      cur_ori_ijk /= cur_ori_ijk.norm();

      // bond status for bond k
      Real bond_status_ijk = _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[k]));

      _local_ke.zero();
      if (coupled_component == 3)
      {
        Real val = cur_ori_ijk(_component) * _bond_dfdT_i_j[cur_nd] / origin_ori_ijk.norm() * vol_k;
        _local_ke(0, cur_nd) += (cur_nd == 0 ? -1 : 1) * val * _bond_status_ij * bond_status_ijk;
        _local_ke(1, cur_nd) += (cur_nd == 0 ? 1 : -1) * val * _bond_status_ij * bond_status_ijk;
      }
      else
      {
        Real val = (cur_nd == 0 ? 1 : -1) * cur_ori_ijk(_component) *
                   _cur_ori_ij(coupled_component) * _bond_dfdU_i_j[cur_nd] / origin_ori_ijk.norm() *
                   vol_k;
        for (_i = 0; _i < _test.size(); ++_i)
          for (_j = 0; _j < _phi.size(); ++_j)
            _local_ke(_i, _j) += (_i == _j ? 1 : -1) * val * _bond_status_ij * bond_status_ijk;
      }

      _assembly.cacheJacobianBlock(_local_ke, ivardofs, jvardofs_ij, _var.scalingFactor());
    }
  }
}
