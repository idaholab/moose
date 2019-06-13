//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmallStrainMechanicsNOSPD.h"
#include "PeridynamicsMesh.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("PeridynamicsApp", SmallStrainMechanicsNOSPD);

template <>
InputParameters
validParams<SmallStrainMechanicsNOSPD>()
{
  InputParameters params = validParams<MechanicsBaseNOSPD>();
  params.addClassDescription(
      "Class for calculating residual and Jacobian for Self-stabilized Non-Ordinary "
      "State-based PeriDynamic (SNOSPD) formulation under small strain assumptions");

  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the variable this kernel acts on (0 for x, 1 for y, 2 for z)");

  return params;
}

SmallStrainMechanicsNOSPD::SmallStrainMechanicsNOSPD(const InputParameters & parameters)
  : MechanicsBaseNOSPD(parameters), _component(getParam<unsigned int>("component"))
{
}

void
SmallStrainMechanicsNOSPD::computeLocalResidual()
{
  // For small strain assumptions, stress measures, i.e., Cauchy stress (Sigma), the first
  // Piola-Kirchhoff stress (P), and the second Piola-Kirchhoff stress (S) are approximately the
  // same. Thus, the nodal force state tensors are calculated using the Cauchy stresses,
  // i.e., T = Sigma * inv(Shape) * xi * multi.
  // Cauchy stress is calculated as Sigma = C * E in the SmallStrainNOSPD material class.

  std::vector<RankTwoTensor> nodal_force(_nnodes);
  for (unsigned int nd = 0; nd < _nnodes; nd++)
    nodal_force[nd] = _stress[nd] * _shape[nd].inverse() * _multi[nd];

  _local_re(0) = -(nodal_force[0].row(_component) + nodal_force[1].row(_component)) *
                 _origin_vec_ij * _bond_status_ij;
  _local_re(1) = -_local_re(0);
}

void
SmallStrainMechanicsNOSPD::computeLocalJacobian()
{
  // excludes dTi/dUj and dTj/dUi contributions, which were considered as nonlocal contribution
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      _local_ke(_i, _j) += (_i == 0 ? -1 : 1) * _multi[_j] *
                           (computeDSDU(_component, _j) * _shape[_j].inverse()).row(_component) *
                           _origin_vec_ij * _bond_status_ij;
}

void
SmallStrainMechanicsNOSPD::computeNonlocalJacobian()
{
  // includes dTi/dUj and dTj/dUi contributions
  // excludes contributions to node i and j, which were considered as local contributions
  for (unsigned int cur_nd = 0; cur_nd < _nnodes; cur_nd++)
  {
    // calculation of jacobian contribution to current_node's neighbors
    std::vector<dof_id_type> dof(_nnodes);
    dof[0] = _current_elem->node_ptr(cur_nd)->dof_number(_sys.number(), _var.number(), 0);
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(cur_nd));
    unsigned int nb =
        std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - cur_nd)) -
        neighbors.begin();
    std::vector<unsigned int> BAneighbors =
        _pdmesh.getBondAssocHorizonNeighbors(_current_elem->node_id(cur_nd), nb);
    std::vector<dof_id_type> bonds = _pdmesh.getAssocBonds(_current_elem->node_id(cur_nd));
    for (unsigned int k = 0; k < BAneighbors.size(); k++)
    {
      const Node * node_k = _pdmesh.nodePtr(neighbors[BAneighbors[k]]);
      dof[1] = node_k->dof_number(_sys.number(), _var.number(), 0);
      const Real vol_k = _pdmesh.getVolume(neighbors[BAneighbors[k]]);

      // obtain bond ik's origin vector
      const RealGradient origin_vec_ijk =
          *node_k - *_pdmesh.nodePtr(_current_elem->node_id(cur_nd));

      RankTwoTensor dFdUk;
      dFdUk.zero();
      for (unsigned int j = 0; j < _dim; j++)
        dFdUk(_component, j) =
            _horizons_ij[cur_nd] / origin_vec_ijk.norm() * origin_vec_ijk(j) * vol_k;

      dFdUk *= _shape[cur_nd].inverse();

      RankTwoTensor dPxdUkx =
          _Jacobian_mult[cur_nd] * 0.5 *
          (dFdUk.transpose() * _dgrad[cur_nd] + _dgrad[cur_nd].transpose() * dFdUk);

      // bond status for bond k
      const Real bond_status_ijk =
          _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[BAneighbors[k]]));

      _local_ke.resize(_test.size(), _phi.size());
      _local_ke.zero();
      for (_i = 0; _i < _test.size(); _i++)
        for (_j = 0; _j < _phi.size(); _j++)
          _local_ke(_i, _j) = (_i == 0 ? -1 : 1) * (_j == 0 ? 0 : 1) * _multi[cur_nd] *
                              (dPxdUkx * _shape[cur_nd].inverse()).row(_component) *
                              _origin_vec_ij * _bond_status_ij * bond_status_ijk;

      _assembly.cacheJacobianBlock(_local_ke, _ivardofs_ij, dof, _var.scalingFactor());

      if (_has_diag_save_in)
      {
        unsigned int rows = _test.size();
        DenseVector<Real> diag(rows);
        for (unsigned int i = 0; i < rows; i++)
          diag(i) = _local_ke(i, i);

        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        for (unsigned int i = 0; i < _diag_save_in.size(); i++)
          _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
      }
    }
  }
}

void
SmallStrainMechanicsNOSPD::computeLocalOffDiagJacobian(unsigned int coupled_component)
{
  _local_ke.zero();
  if (coupled_component == 3) // temperature is coupled
  {
    std::vector<RankTwoTensor> dSdT(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; nd++)
      for (unsigned int es = 0; es < _deigenstrain_dT.size(); es++)
        dSdT[nd] = -_Jacobian_mult[nd] * (*_deigenstrain_dT[es])[nd];

    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) += (_i == 0 ? -1 : 1) * _multi[_j] *
                             (dSdT[_j] * _shape[_j].inverse()).row(_component) * _origin_vec_ij *
                             _bond_status_ij;
  }
  else if (coupled_component == 4) // weak plane stress case, out_of_plane_strain is coupled
  {
    std::vector<RankTwoTensor> dSdE33(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; nd++)
      for (unsigned int i = 0; i < 3; i++)
        for (unsigned int j = 0; j < 3; j++)
          dSdE33[nd](i, j) = _Jacobian_mult[nd](i, j, 2, 2);

    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) += (_i == 0 ? -1 : 1) * _multi[_j] *
                             (dSdE33[_j] * _shape[_j].inverse()).row(_component) * _origin_vec_ij *
                             _bond_status_ij;
  }
  else // off-diagonal Jacobian with respect to other displacement variables
  {
    // ONLY consider the contributions to node i and j
    // contributions to their neighbors are considered as nonlocal off-diagonal
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) +=
            (_i == 0 ? -1 : 1) * _multi[_j] *
            (computeDSDU(coupled_component, _j) * _shape[_j].inverse()).row(_component) *
            _origin_vec_ij * _bond_status_ij;
  }
}

void
SmallStrainMechanicsNOSPD::computePDNonlocalOffDiagJacobian(unsigned int jvar_num,
                                                            unsigned int coupled_component)
{
  if (coupled_component != 3 && coupled_component != 4)
  {
    for (unsigned int cur_nd = 0; cur_nd < _nnodes; cur_nd++)
    {
      // calculation of jacobian contribution to current_node's neighbors
      // NOT including the contribution to nodes i and j, which is considered as local off-diagonal
      std::vector<dof_id_type> jvardofs_ijk(_nnodes);
      jvardofs_ijk[0] = _current_elem->node_ptr(cur_nd)->dof_number(_sys.number(), jvar_num, 0);
      std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(cur_nd));
      unsigned int nb =
          std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - cur_nd)) -
          neighbors.begin();
      std::vector<unsigned int> BAneighbors =
          _pdmesh.getBondAssocHorizonNeighbors(_current_elem->node_id(cur_nd), nb);
      std::vector<dof_id_type> bonds = _pdmesh.getAssocBonds(_current_elem->node_id(cur_nd));
      for (unsigned int k = 0; k < BAneighbors.size(); k++)
      {
        const Node * node_k = _pdmesh.nodePtr(neighbors[BAneighbors[k]]);
        jvardofs_ijk[1] = node_k->dof_number(_sys.number(), jvar_num, 0);
        const Real vol_k = _pdmesh.getVolume(neighbors[BAneighbors[k]]);

        // obtain bond k's origin vector
        const RealGradient origin_vec_ijk =
            *node_k - *_pdmesh.nodePtr(_current_elem->node_id(cur_nd));

        RankTwoTensor dFdUk;
        dFdUk.zero();
        for (unsigned int j = 0; j < _dim; j++)
          dFdUk(coupled_component, j) =
              _horizons_ij[cur_nd] / origin_vec_ijk.norm() * origin_vec_ijk(j) * vol_k;

        dFdUk *= _shape[cur_nd].inverse();

        RankTwoTensor dPxdUky =
            _Jacobian_mult[cur_nd] * 0.5 *
            (dFdUk.transpose() * _dgrad[cur_nd] + _dgrad[cur_nd].transpose() * dFdUk);

        // bond status for bond k
        const Real bond_status_ijk =
            _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[BAneighbors[k]]));

        _local_ke.zero();
        for (_i = 0; _i < _test.size(); _i++)
          for (_j = 0; _j < _phi.size(); _j++)
            _local_ke(_i, _j) = (_i == 0 ? -1 : 1) * (_j == 0 ? 0 : 1) * _multi[cur_nd] *
                                (dPxdUky * _shape[cur_nd].inverse()).row(_component) *
                                _origin_vec_ij * _bond_status_ij * bond_status_ijk;

        _assembly.cacheJacobianBlock(_local_ke, _ivardofs_ij, jvardofs_ijk, _var.scalingFactor());
      }
    }
  }
}
