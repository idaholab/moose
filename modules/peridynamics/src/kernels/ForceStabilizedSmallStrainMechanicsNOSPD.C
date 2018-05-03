//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ForceStabilizedSmallStrainMechanicsNOSPD.h"
#include "MeshBasePD.h"

registerMooseObject("PeridynamicsApp", ForceStabilizedSmallStrainMechanicsNOSPD);

template <>
InputParameters
validParams<ForceStabilizedSmallStrainMechanicsNOSPD>()
{
  InputParameters params = validParams<MechanicsBasePD>();
  params.addClassDescription("Class for calculating residual and Jacobian for Non-Ordinary "
                             "State-based PeriDynamic solid mechanics formulation");

  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the variable this kernel acts on (0 for x, 1 for y, 2 for z)");

  return params;
}

ForceStabilizedSmallStrainMechanicsNOSPD::ForceStabilizedSmallStrainMechanicsNOSPD(
    const InputParameters & parameters)
  : MechanicsBasePD(parameters),
    _multi(getMaterialProperty<Real>("multi")),
    _stress(getMaterialProperty<RankTwoTensor>("stress")),
    _shape(getMaterialProperty<RankTwoTensor>("shape_tensor")),
    _dgrad(getMaterialProperty<RankTwoTensor>("deformation_gradient")),
    _ddgraddu(getMaterialProperty<RankTwoTensor>("ddeformation_gradient_du")),
    _ddgraddv(getMaterialProperty<RankTwoTensor>("ddeformation_gradient_dv")),
    _ddgraddw(getMaterialProperty<RankTwoTensor>("ddeformation_gradient_dw")),
    _Cijkl(getMaterialProperty<RankFourTensor>("elasticity_tensor")),
    _sf_coeff(getMaterialProperty<Real>("stabilization_force_coeff")),
    _component(getParam<unsigned int>("component"))
{
}

void
ForceStabilizedSmallStrainMechanicsNOSPD::computeLocalResidual()
{
  Real sforce = 0.5 * (_sf_coeff[0] + _sf_coeff[1]) *
                (_disp_var[_component]->getNodalValue(*_nodes_ij[1]) -
                 _disp_var[_component]->getNodalValue(*_nodes_ij[0]));

  // For small strain assumptions, stress measures, i.e., Cauchy stress (Sigma), the first
  // Piola-Kirchhoff stress (P), and the second Piola-Kirchhoff stress (S) are approximately the
  // same. Thus, the nodal force state tensors are calculated using the Cauchy stresses,
  // i.e., T = Sigma * inv(Shape) * xi * multi.
  // Cauchy stress is calculated as Sigma = C * E in the SmallStrainNOSPD material class.

  std::vector<RankTwoTensor> nodal_force(_nnodes);
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
    nodal_force[nd] = _stress[nd] * _shape[nd].inverse() * _multi[nd];

  _local_re(0) =
      -((nodal_force[0].row(_component) + nodal_force[1].row(_component)) * _origin_vec_ij +
        sforce) *
      _bond_status_ij;

  _local_re(1) = -_local_re(0);
}

void
ForceStabilizedSmallStrainMechanicsNOSPD::computeLocalJacobian()
{
  // excludes dTi/dUj and dTj/dUi contribution which was considered as nonlocal contribution
  for (_i = 0; _i < _test.size(); ++_i)
    for (_j = 0; _j < _phi.size(); ++_j)
      _local_ke(_i, _j) += (_i == 0 ? -1 : 1) * _multi[_j] *
                           (computeDSDU(_component, _j) * _shape[_j].inverse()).row(_component) *
                           _origin_vec_ij * _bond_status_ij;

  // compute local stabilization force jacobian
  Real diag = 0.5 * (_sf_coeff[0] + _sf_coeff[1]);
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      _local_ke(_i, _j) += (_i == _j ? 1 : -1) * diag * _bond_status_ij;
}

void
ForceStabilizedSmallStrainMechanicsNOSPD::computeNonlocalJacobian()
{
  // includes dTi/dUj and dTj/dUi contributions
  for (unsigned int cur_nd = 0; cur_nd < _nnodes; ++cur_nd)
  {
    // calculation of jacobian contribution to current_node's neighbors
    std::vector<dof_id_type> dof(_nnodes);
    dof[0] = _nodes_ij[cur_nd]->dof_number(_sys.number(), _var.number(), 0);
    std::vector<dof_id_type> neighbors = _pdmesh.neighbors(_nodes_ij[cur_nd]->id());
    std::vector<dof_id_type> bonds = _pdmesh.bonds(_nodes_ij[cur_nd]->id());
    for (unsigned int k = 0; k < neighbors.size(); ++k)
    {
      Node * node_k = _pdmesh.nodePtr(neighbors[k]);
      dof[1] = node_k->dof_number(_sys.number(), _var.number(), 0);
      Real vol_k = _pdmesh.volume(neighbors[k]);

      // obtain bond ik's origin vector
      RealGradient origin_vec_ijk(_dim);
      for (unsigned int j = 0; j < _dim; ++j)
        origin_vec_ijk(j) =
            _pdmesh.coord(neighbors[k])(j) - _pdmesh.coord(_nodes_ij[cur_nd]->id())(j);

      RankTwoTensor dFdUk;
      dFdUk.zero();
      for (unsigned int j = 0; j < _dim; ++j)
        dFdUk(_component, j) =
            vol_k * _horizons_ij[cur_nd] / origin_vec_ijk.norm() * origin_vec_ijk(j);

      dFdUk *= _shape[cur_nd].inverse();
      RankTwoTensor dSxdUkx =
          _Cijkl[cur_nd] * 0.5 *
          (dFdUk.transpose() * _dgrad[cur_nd] + _dgrad[cur_nd].transpose() * dFdUk);

      // bond status for bond k
      Real bond_status_ijk = _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[k]));

      _local_ke.resize(_test.size(), _phi.size());
      _local_ke.zero();
      for (_i = 0; _i < _test.size(); ++_i)
        for (_j = 0; _j < _phi.size(); ++_j)
          _local_ke(_i, _j) = (_i == 0 ? -1 : 1) * (_j == 0 ? 0 : 1) * _multi[cur_nd] *
                              (dSxdUkx * _shape[cur_nd].inverse()).row(_component) *
                              _origin_vec_ij * _bond_status_ij * bond_status_ijk;

      _assembly.cacheJacobianBlock(_local_ke, _ivardofs_ij, dof, _var.scalingFactor());

      if (_has_diag_save_in)
      {
        unsigned int rows = _test.size();
        DenseVector<Real> diag(rows);
        for (unsigned int i = 0; i < rows; ++i)
          diag(i) = _local_ke(i, i);

        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
          _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
      }
    }
  }
}

void
ForceStabilizedSmallStrainMechanicsNOSPD::computeLocalOffDiagJacobian(
    unsigned int coupled_component)
{
  _local_ke.zero();
  if (coupled_component == 3)
  {
    std::vector<RankTwoTensor> dSdT(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
      for (unsigned int es = 0; es < _deigenstrain_dT.size(); ++es)
        dSdT[nd] = -(_Cijkl[nd] * (*_deigenstrain_dT[es])[nd]);

    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < _phi.size(); ++_j)
        _local_ke(_i, _j) += (_i == 0 ? -1 : 1) * _multi[_j] *
                             (dSdT[_j] * _shape[_j].inverse()).row(_component) * _origin_vec_ij *
                             _bond_status_ij;
  }
  else
  {
    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < _phi.size(); ++_j)
        _local_ke(_i, _j) +=
            (_i == 0 ? -1 : 1) * _multi[_j] *
            (computeDSDU(coupled_component, _j) * _shape[_j].inverse()).row(_component) *
            _origin_vec_ij * _bond_status_ij;
  }
}

void
ForceStabilizedSmallStrainMechanicsNOSPD::computePDNonlocalOffDiagJacobian(
    unsigned int jvar_num, unsigned int coupled_component)
{
  if (coupled_component != 3)
  {
    for (unsigned int cur_nd = 0; cur_nd < _nnodes; ++cur_nd)
    {
      // calculation of jacobian contribution to current_node's neighbors
      std::vector<dof_id_type> jvardofs_ijk(_nnodes);
      jvardofs_ijk[0] = _nodes_ij[cur_nd]->dof_number(_sys.number(), jvar_num, 0);
      std::vector<dof_id_type> neighbors = _pdmesh.neighbors(_nodes_ij[cur_nd]->id());
      std::vector<dof_id_type> bonds = _pdmesh.bonds(_nodes_ij[cur_nd]->id());
      for (unsigned int k = 0; k < neighbors.size(); ++k)
      {
        Node * node_k = _pdmesh.nodePtr(neighbors[k]);
        jvardofs_ijk[1] = node_k->dof_number(_sys.number(), jvar_num, 0);
        Real vol_k = _pdmesh.volume(neighbors[k]);

        // obtain bond k's origin vector
        RealGradient origin_vec_ijk(_dim);
        for (unsigned int j = 0; j < _dim; ++j)
          origin_vec_ijk(j) =
              _pdmesh.coord(neighbors[k])(j) - _pdmesh.coord(_nodes_ij[cur_nd]->id())(j);

        RankTwoTensor dFdUk;
        dFdUk.zero();
        for (unsigned int j = 0; j < _dim; ++j)
          dFdUk(coupled_component, j) =
              _horizons_ij[cur_nd] / origin_vec_ijk.norm() * origin_vec_ijk(j) * vol_k;

        dFdUk *= _shape[cur_nd].inverse();
        RankTwoTensor dSxdUky =
            _Cijkl[cur_nd] * 0.5 *
            (dFdUk.transpose() * _dgrad[cur_nd] + _dgrad[cur_nd].transpose() * dFdUk);

        // bond status for bond k
        Real bond_status_ijk = _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[k]));

        _local_ke.zero();
        for (_i = 0; _i < _test.size(); ++_i)
          for (_j = 0; _j < _phi.size(); ++_j)
            _local_ke(_i, _j) = (_i == 0 ? -1 : 1) * (_j == 0 ? 0 : 1) * _multi[cur_nd] *
                                (dSxdUky * _shape[cur_nd].inverse()).row(_component) *
                                _origin_vec_ij * _bond_status_ij * bond_status_ijk;

        _assembly.cacheJacobianBlock(_local_ke, _ivardofs_ij, jvardofs_ijk, _var.scalingFactor());
      }
    }
  }
}

RankTwoTensor
ForceStabilizedSmallStrainMechanicsNOSPD::computeDSDU(unsigned int component, unsigned int nd)
{
  // compute the derivative of stress w.r.t the solution components
  RankTwoTensor dSdU;
  if (component == 0)
    dSdU = _Cijkl[nd] * 0.5 *
           (_ddgraddu[nd].transpose() * _dgrad[nd] + _dgrad[nd].transpose() * _ddgraddu[nd]);
  else if (component == 1)
    dSdU = _Cijkl[nd] * 0.5 *
           (_ddgraddv[nd].transpose() * _dgrad[nd] + _dgrad[nd].transpose() * _ddgraddv[nd]);
  else if (component == 2)
    dSdU = _Cijkl[nd] * 0.5 *
           (_ddgraddw[nd].transpose() * _dgrad[nd] + _dgrad[nd].transpose() * _ddgraddw[nd]);

  return dSdU;
}
