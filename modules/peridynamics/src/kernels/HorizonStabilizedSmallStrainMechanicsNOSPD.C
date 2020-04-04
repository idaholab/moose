//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HorizonStabilizedSmallStrainMechanicsNOSPD.h"
#include "PeridynamicsMesh.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("PeridynamicsApp", HorizonStabilizedSmallStrainMechanicsNOSPD);

InputParameters
HorizonStabilizedSmallStrainMechanicsNOSPD::validParams()
{
  InputParameters params = MechanicsBaseNOSPD::validParams();
  params.addClassDescription(
      "Class for calculating the residual and Jacobian for the horizon-stabilized non-ordinary "
      "state-based peridynamic correspondence model under small strain assumptions");

  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the variable this kernel acts on (0 for x, 1 for y, 2 for z)");

  return params;
}

HorizonStabilizedSmallStrainMechanicsNOSPD::HorizonStabilizedSmallStrainMechanicsNOSPD(
    const InputParameters & parameters)
  : MechanicsBaseNOSPD(parameters), _component(getParam<unsigned int>("component"))
{
}

void
HorizonStabilizedSmallStrainMechanicsNOSPD::computeLocalResidual()
{
  // For small strain assumptions, stress measures, i.e., Cauchy stress (Sigma), the first
  // Piola-Kirchhoff stress (P), and the second Piola-Kirchhoff stress (S) are approximately the
  // same. Thus, the nodal force state tensors are calculated using the Cauchy stresses,
  // i.e., T = Sigma * inv(Shape) * xi * multi.
  // Cauchy stress is calculated as Sigma = C * E in the ComputeSmallStrainNOSPD material class.

  std::vector<Real> nodal_force_comp(_nnodes);
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
    nodal_force_comp[nd] = _multi[nd] * (_stress[nd] * _shape2[nd].inverse()).row(_component) *
                           (nd == 0 ? 1 : -1) * _origin_vec;

  _local_re(0) = -(nodal_force_comp[0] - nodal_force_comp[1]) * _bond_status;
  _local_re(1) = -_local_re(0);
}

void
HorizonStabilizedSmallStrainMechanicsNOSPD::computeLocalJacobian()
{
  // excludes dTi/dUj and dTj/dUi contributions, which were considered as nonlocal contribution
  for (_i = 0; _i < _test.size(); ++_i)
    for (_j = 0; _j < _phi.size(); ++_j)
      _local_ke(_i, _j) += (_i == 0 ? -1 : 1) * _multi[_j] *
                           (computeDSDU(_component, _j) * _shape2[_j].inverse()).row(_component) *
                           _origin_vec * _bond_status;
}

void
HorizonStabilizedSmallStrainMechanicsNOSPD::computeNonlocalJacobian()
{
  // includes dTi/dUj and dTj/dUi contributions
  // excludes contributions to node i and j, which were considered as local contributions
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    // calculation of jacobian contribution to current_node's neighbors
    std::vector<dof_id_type> dof(_nnodes);
    dof[0] = _current_elem->node_ptr(nd)->dof_number(_sys.number(), _var.number(), 0);
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));
    // get deformation gradient neighbors
    unsigned int nb_index =
        std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - nd)) -
        neighbors.begin();
    std::vector<dof_id_type> dg_neighbors =
        _pdmesh.getDefGradNeighbors(_current_elem->node_id(nd), nb_index);

    Real vol_nb;
    RealGradient origin_vec_nb;
    RankTwoTensor dFdUk, dPxdUkx;

    for (unsigned int nb = 0; nb < dg_neighbors.size(); ++nb)
      if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[nb]])) > 0.5)
      {
        Node * dgneighbor_i = _pdmesh.nodePtr(neighbors[dg_neighbors[nb]]);
        dof[1] = dgneighbor_i->dof_number(_sys.number(), _var.number(), 0);
        vol_nb = _pdmesh.getPDNodeVolume(neighbors[dg_neighbors[nb]]);

        // obtain bond ind's origin vector
        origin_vec_nb = *dgneighbor_i - *_pdmesh.nodePtr(_current_elem->node_id(nd));

        dFdUk.zero();
        for (unsigned int k = 0; k < _dim; ++k)
          dFdUk(_component, k) = _horiz_rad[nd] / origin_vec_nb.norm() * origin_vec_nb(k) * vol_nb;

        dFdUk *= _shape2[nd].inverse();

        dPxdUkx = _Jacobian_mult[nd] * 0.5 * (dFdUk.transpose() + dFdUk);

        _local_ke.resize(_test.size(), _phi.size());
        _local_ke.zero();
        for (_i = 0; _i < _test.size(); ++_i)
          for (_j = 0; _j < _phi.size(); ++_j)
            _local_ke(_i, _j) = (_i == 0 ? -1 : 1) * (_j == 0 ? 0 : 1) * _multi[nd] *
                                (dPxdUkx * _shape2[nd].inverse()).row(_component) * _origin_vec *
                                _bond_status;

        _assembly.cacheJacobianBlock(_local_ke, _ivardofs, dof, _var.scalingFactor());

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
HorizonStabilizedSmallStrainMechanicsNOSPD::computeLocalOffDiagJacobian(
    unsigned int coupled_component)
{
  _local_ke.zero();
  if (coupled_component == 3) // temperature is coupled
  {
    std::vector<RankTwoTensor> dSdT(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
      for (unsigned int es = 0; es < _deigenstrain_dT.size(); ++es)
        dSdT[nd] = -_Jacobian_mult[nd] * (*_deigenstrain_dT[es])[nd];

    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < _phi.size(); ++_j)
        _local_ke(_i, _j) += (_i == 0 ? -1 : 1) * _multi[_j] *
                             (dSdT[_j] * _shape2[_j].inverse()).row(_component) * _origin_vec *
                             _bond_status;
  }
  else if (coupled_component == 4) // weak plane stress case, out_of_plane_strain is coupled
  {
    std::vector<RankTwoTensor> dSdE33(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
      for (_i = 0; _i < 3; ++_i)
        for (_j = 0; _j < 3; ++_j)
          dSdE33[nd](_i, _j) = _Jacobian_mult[nd](_i, _j, 2, 2);

    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < _phi.size(); ++_j)
        _local_ke(_i, _j) += (_i == 0 ? -1 : 1) * _multi[_j] *
                             (dSdE33[_j] * _shape2[_j].inverse()).row(_component) * _origin_vec *
                             _bond_status;
  }
  else // off-diagonal Jacobian with respect to other displacement variables
  {
    // ONLY consider the contributions to node i and j
    // contributions to their neighbors are considered as nonlocal off-diagonal
    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < _phi.size(); ++_j)
        _local_ke(_i, _j) +=
            (_i == 0 ? -1 : 1) * _multi[_j] *
            (computeDSDU(coupled_component, _j) * _shape2[_j].inverse()).row(_component) *
            _origin_vec * _bond_status;
  }
}

void
HorizonStabilizedSmallStrainMechanicsNOSPD::computePDNonlocalOffDiagJacobian(
    unsigned int jvar_num, unsigned int coupled_component)
{
  if (coupled_component != 3 && coupled_component != 4)
  {
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
    {
      // calculation of jacobian contribution to current_node's neighbors
      // NOT including the contribution to nodes i and j, which is considered as local off-diagonal
      std::vector<dof_id_type> jvardofs(_nnodes);
      jvardofs[0] = _current_elem->node_ptr(nd)->dof_number(_sys.number(), jvar_num, 0);
      std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
      std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));
      // get deformation gradient neighbors
      unsigned int nb_index =
          std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - nd)) -
          neighbors.begin();
      std::vector<dof_id_type> dg_neighbors =
          _pdmesh.getDefGradNeighbors(_current_elem->node_id(nd), nb_index);

      Real vol_nb;
      RealGradient origin_vec_nb;
      RankTwoTensor dFdUk, dPxdUky;

      for (unsigned int nb = 0; nb < dg_neighbors.size(); ++nb)
        if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[nb]])) > 0.5)

        {
          Node * dgneighbor_i = _pdmesh.nodePtr(neighbors[dg_neighbors[nb]]);
          jvardofs[1] = dgneighbor_i->dof_number(_sys.number(), jvar_num, 0);
          vol_nb = _pdmesh.getPDNodeVolume(neighbors[dg_neighbors[nb]]);

          // obtain bond ndnb's origin vector
          origin_vec_nb = *dgneighbor_i - *_pdmesh.nodePtr(_current_elem->node_id(nd));

          dFdUk.zero();
          for (unsigned int j = 0; j < _dim; ++j)
            dFdUk(coupled_component, j) =
                _horiz_rad[nd] / origin_vec_nb.norm() * origin_vec_nb(j) * vol_nb;

          dFdUk *= _shape2[nd].inverse();

          dPxdUky = _Jacobian_mult[nd] * 0.5 * (dFdUk.transpose() + dFdUk);

          _local_ke.zero();
          for (_i = 0; _i < _test.size(); ++_i)
            for (_j = 0; _j < _phi.size(); ++_j)
              _local_ke(_i, _j) = (_i == 0 ? -1 : 1) * (_j == 0 ? 0 : 1) * _multi[nd] *
                                  (dPxdUky * _shape2[nd].inverse()).row(_component) * _origin_vec *
                                  _bond_status;

          _assembly.cacheJacobianBlock(_local_ke, _ivardofs, jvardofs, _var.scalingFactor());
        }
    }
  }
}
