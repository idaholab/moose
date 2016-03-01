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

InputParameters
MechanicsOSPD::validParams()
{
  InputParameters params = MechanicsBasePD::validParams();
  params.addClassDescription(
      "Class for calculating the residual and Jacobian for the ordinary state-based "
      "peridynamic mechanics formulation");

  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the variable this kernel acts on (0 for x, 1 for y, 2 for z)");

  return params;
}

MechanicsOSPD::MechanicsOSPD(const InputParameters & parameters)
  : MechanicsBasePD(parameters),
    _bond_local_force(getMaterialProperty<Real>("bond_local_force")),
    _bond_nonlocal_force(getMaterialProperty<Real>("bond_nonlocal_force")),
    _bond_local_dfdU(getMaterialProperty<Real>("bond_dfdU")),
    _bond_nonlocal_dfdU(getMaterialProperty<Real>("bond_nonlocal_dfdU")),
    _bond_local_dfdT(getMaterialProperty<Real>("bond_dfdT")),
    _bond_nonlocal_dfdT(getMaterialProperty<Real>("bond_nonlocal_dfdT")),
    _component(getParam<unsigned int>("component"))
{
}

void
MechanicsOSPD::computeLocalResidual()
{
  // H term
  _local_re(0) = -_bond_local_force[0] * _current_unit_vec(_component) * _bond_status;
  _local_re(1) = -_local_re(0);
}

void
MechanicsOSPD::computeNonlocalResidual()
{
  // P and Q terms
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    // calculation of residual contribution to current_node's neighbors
    std::vector<dof_id_type> ivardofs(_nnodes);
    ivardofs[nd] = _ivardofs[nd];
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

    Real vol_nb;
    RealGradient origin_vec_nb, current_vec_nb;

    for (unsigned int nb = 0; nb < neighbors.size(); ++nb)
      if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)
      {
        ivardofs[1 - nd] =
            _pdmesh.nodePtr(neighbors[nb])->dof_number(_sys.number(), _var.number(), 0);
        vol_nb = _pdmesh.getNodeVolume(neighbors[nb]);

        origin_vec_nb =
            *_pdmesh.nodePtr(neighbors[nb]) - *_pdmesh.nodePtr(_current_elem->node_id(nd));

        for (unsigned int i = 0; i < _dim; ++i)
          current_vec_nb(i) = origin_vec_nb(i) +
                              _disp_var[i]->getNodalValue(*_pdmesh.nodePtr(neighbors[nb])) -
                              _disp_var[i]->getNodalValue(*_current_elem->node_ptr(nd));

        current_vec_nb /= current_vec_nb.norm();

        _local_re(0) = (nd == 0 ? -1 : 1) * _bond_nonlocal_force[nd] * vol_nb /
                       origin_vec_nb.norm() * current_vec_nb(_component) * _bond_status;
        _local_re(1) = -_local_re(0);

        // cache the residual contribution to node_i and its neighbor nb using their global dof
        // indices
        _assembly.cacheResidualNodes(_local_re, ivardofs);

        // save in the displacement residuals
        if (_has_save_in)
        {
          Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
          for (unsigned int i = 0; i < _save_in.size(); ++i)
          {
            std::vector<dof_id_type> save_in_dofs(2);
            save_in_dofs[nd] = _current_elem->node_ptr(nd)->dof_number(
                _save_in[i]->sys().number(), _save_in[i]->number(), 0);
            save_in_dofs[1 - nd] =
                _pdmesh.nodePtr(neighbors[nb])
                    ->dof_number(_save_in[i]->sys().number(), _save_in[i]->number(), 0);

            _save_in[i]->sys().solution().add_vector(_local_re, save_in_dofs);
          }
        }
      }
  }
}

void
MechanicsOSPD::computeLocalJacobian()
{
  const Real val =
      _current_unit_vec(_component) * _current_unit_vec(_component) * _bond_local_dfdU[0] +
      _bond_local_force[0] * (1.0 - _current_unit_vec(_component) * _current_unit_vec(_component)) /
          _current_vec.norm();

  for (unsigned int i = 0; i < _nnodes; ++i)
    for (unsigned int j = 0; j < _nnodes; ++j)
      _local_ke(i, j) += (i == j ? 1 : -1) * val * _bond_status;
}

void
MechanicsOSPD::computeLocalOffDiagJacobian(unsigned int jvar_num, unsigned int coupled_component)
{
  if (_temp_coupled && jvar_num == _temp_var->number())
  {
    for (unsigned int i = 0; i < _nnodes; ++i)
      for (unsigned int j = 0; j < _nnodes; ++j)
        _local_ke(i, j) +=
            (i == 1 ? 1 : -1) * _current_unit_vec(_component) * _bond_local_dfdT[0] * _bond_status;
  }
  else
  {
    const Real val =
        _current_unit_vec(_component) * _current_unit_vec(coupled_component) * _bond_local_dfdU[0] -
        _bond_local_force[0] * _current_unit_vec(_component) *
            _current_unit_vec(coupled_component) / _current_vec.norm();

    for (unsigned int i = 0; i < _nnodes; ++i)
      for (unsigned int j = 0; j < _nnodes; ++j)
        _local_ke(i, j) += (i == j ? 1 : -1) * val * _bond_status;
  }
}

void
MechanicsOSPD::computeNonlocalJacobian()
{
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    // calculation of jacobian contribution to current_node's neighbors
    std::vector<dof_id_type> ivardofs(_nnodes);
    ivardofs[nd] = _ivardofs[nd];
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

    Real vol_nb;
    RealGradient origin_vec_nb, current_vec_nb;

    for (unsigned int nb = 0; nb < neighbors.size(); ++nb)
      if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)
      {
        ivardofs[1 - nd] =
            _pdmesh.nodePtr(neighbors[nb])->dof_number(_sys.number(), _var.number(), 0);
        vol_nb = _pdmesh.getNodeVolume(neighbors[nb]);

        origin_vec_nb =
            *_pdmesh.nodePtr(neighbors[nb]) - *_pdmesh.nodePtr(_current_elem->node_id(nd));

        for (unsigned int k = 0; k < _dim; ++k)
          current_vec_nb(k) = origin_vec_nb(k) +
                              _disp_var[k]->getNodalValue(*_pdmesh.nodePtr(neighbors[nb])) -
                              _disp_var[k]->getNodalValue(*_current_elem->node_ptr(nd));

        current_vec_nb /= current_vec_nb.norm();

        const Real val = (nd == 0 ? 1 : -1) * current_vec_nb(_component) *
                         _current_unit_vec(_component) * _bond_nonlocal_dfdU[nd];

        _local_ke.zero();
        for (unsigned int i = 0; i < _nnodes; ++i)
          for (unsigned int j = 0; j < _nnodes; ++j)
            _local_ke(i, j) +=
                (i == j ? 1 : -1) * val / origin_vec_nb.norm() * vol_nb * _bond_status;

        _assembly.cacheJacobianBlock(_local_ke, ivardofs, _ivardofs, _var.scalingFactor());

        if (_has_diag_save_in)
        {
          unsigned int rows = _nnodes;
          DenseVector<Real> diag(rows);
          for (unsigned int i = 0; i < rows; ++i)
            diag(i) = _local_ke(i, i);

          diag(1 - nd) = 0;

          Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
          for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
          {
            std::vector<dof_id_type> diag_save_in_dofs(2);
            diag_save_in_dofs[nd] = _current_elem->node_ptr(nd)->dof_number(
                _diag_save_in[i]->sys().number(), _diag_save_in[i]->number(), 0);
            diag_save_in_dofs[1 - nd] =
                _pdmesh.nodePtr(neighbors[nb])
                    ->dof_number(_diag_save_in[i]->sys().number(), _diag_save_in[i]->number(), 0);

            _diag_save_in[i]->sys().solution().add_vector(diag, diag_save_in_dofs);
          }
        }

        const Real val2 = _bond_nonlocal_force[nd] *
                          (1.0 - current_vec_nb(_component) * current_vec_nb(_component)) /
                          current_vec_nb.norm();

        _local_ke.zero();
        for (unsigned int i = 0; i < _nnodes; ++i)
          for (unsigned int j = 0; j < _nnodes; ++j)
            _local_ke(i, j) +=
                (i == j ? 1 : -1) * val2 / origin_vec_nb.norm() * vol_nb * _bond_status;

        _assembly.cacheJacobianBlock(_local_ke, ivardofs, ivardofs, _var.scalingFactor());

        if (_has_diag_save_in)
        {
          unsigned int rows = _nnodes;
          DenseVector<Real> diag(rows);
          for (unsigned int i = 0; i < rows; ++i)
            diag(i) = _local_ke(i, i);

          Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
          for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
          {
            std::vector<dof_id_type> diag_save_in_dofs(2);
            diag_save_in_dofs[nd] = _current_elem->node_ptr(nd)->dof_number(
                _diag_save_in[i]->sys().number(), _diag_save_in[i]->number(), 0);
            diag_save_in_dofs[1 - nd] =
                _pdmesh.nodePtr(neighbors[nb])
                    ->dof_number(_diag_save_in[i]->sys().number(), _diag_save_in[i]->number(), 0);

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

  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    // calculation of jacobian contribution to current_node's neighbors
    std::vector<dof_id_type> ivardofs(_nnodes), jvardofs(_nnodes);
    ivardofs[nd] = _ivardofs[nd];
    jvardofs[nd] = jvardofs_ij[nd];
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

    Real vol_nb;
    RealGradient origin_vec_nb, current_vec_nb;

    for (unsigned int nb = 0; nb < neighbors.size(); ++nb)
      if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)
      {
        ivardofs[1 - nd] =
            _pdmesh.nodePtr(neighbors[nb])->dof_number(_sys.number(), _var.number(), 0);
        jvardofs[1 - nd] = _pdmesh.nodePtr(neighbors[nb])->dof_number(_sys.number(), jvar_num, 0);
        vol_nb = _pdmesh.getNodeVolume(neighbors[nb]);

        origin_vec_nb =
            *_pdmesh.nodePtr(neighbors[nb]) - *_pdmesh.nodePtr(_current_elem->node_id(nd));

        for (unsigned int i = 0; i < _dim; ++i)
          current_vec_nb(i) = origin_vec_nb(i) +
                              _disp_var[i]->getNodalValue(*_pdmesh.nodePtr(neighbors[nb])) -
                              _disp_var[i]->getNodalValue(*_current_elem->node_ptr(nd));

        current_vec_nb /= current_vec_nb.norm();

        _local_ke.zero();
        if (_temp_coupled && jvar_num == _temp_var->number())
        {
          const Real val =
              current_vec_nb(_component) * _bond_nonlocal_dfdT[nd] / origin_vec_nb.norm() * vol_nb;

          _local_ke(0, nd) += (nd == 0 ? -1 : 1) * val * _bond_status;
          _local_ke(1, nd) += (nd == 0 ? 1 : -1) * val * _bond_status;
        }
        else
        {
          const Real val = (nd == 0 ? 1 : -1) * current_vec_nb(_component) *
                           _current_unit_vec(coupled_component) * _bond_nonlocal_dfdU[nd] /
                           origin_vec_nb.norm() * vol_nb;

          for (unsigned int i = 0; i < _nnodes; ++i)
            for (unsigned int j = 0; j < _nnodes; ++j)
              _local_ke(i, j) += (i == j ? 1 : -1) * val * _bond_status;
        }

        _assembly.cacheJacobianBlock(_local_ke, ivardofs, jvardofs_ij, _var.scalingFactor());
      }
  }
}
