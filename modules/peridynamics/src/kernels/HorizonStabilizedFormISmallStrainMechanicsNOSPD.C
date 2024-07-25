//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HorizonStabilizedFormISmallStrainMechanicsNOSPD.h"
#include "PeridynamicsMesh.h"

registerMooseObject("PeridynamicsApp", HorizonStabilizedFormISmallStrainMechanicsNOSPD);

InputParameters
HorizonStabilizedFormISmallStrainMechanicsNOSPD::validParams()
{
  InputParameters params = MechanicsBaseNOSPD::validParams();
  params.addClassDescription(
      "Class for calculating the residual and the Jacobian for Form I of the horizon-stabilized"
      "peridynamic correspondence model under small strain assumptions");

  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the variable this kernel acts on (0 for x, 1 for y, 2 for z)");

  return params;
}

HorizonStabilizedFormISmallStrainMechanicsNOSPD::HorizonStabilizedFormISmallStrainMechanicsNOSPD(
    const InputParameters & parameters)
  : MechanicsBaseNOSPD(parameters), _component(getParam<unsigned int>("component"))
{
}

void
HorizonStabilizedFormISmallStrainMechanicsNOSPD::computeLocalResidual()
{
  // For small strain assumptions, stress measures, i.e., Cauchy stress (Sigma), the first
  // Piola-Kirchhoff stress (P), and the second Piola-Kirchhoff stress (S) are approximately the
  // same. Thus, the nodal force state tensors are calculated using the Cauchy stresses,
  // i.e., T = Sigma * inv(Shape) * xi * multi.
  // Cauchy stress is calculated as Sigma = C * E in the ComputeSmallStrainNOSPD material class.

  for (unsigned int nd = 0; nd < _nnodes; ++nd)
    for (unsigned int i = 0; i < _nnodes; ++i)
      _local_re(i) += (i == 0 ? -1 : 1) * _multi[nd] *
                      (_stress[nd] * _shape2[nd].inverse()).row(_component) * _origin_vec *
                      _bond_status;
}

void
HorizonStabilizedFormISmallStrainMechanicsNOSPD::computeLocalJacobian()
{
  // excludes dTi/dUj and dTj/dUi contributions, which were considered as nonlocal contribution
  for (unsigned int i = 0; i < _nnodes; ++i)
    for (unsigned int j = 0; j < _nnodes; ++j)
      _local_ke(i, j) += (i == 0 ? -1 : 1) * _multi[j] *
                         (computeDSDU(_component, j) * _shape2[j].inverse()).row(_component) *
                         _origin_vec * _bond_status;
}

void
HorizonStabilizedFormISmallStrainMechanicsNOSPD::computeNonlocalJacobian()
{
  // includes dTi/dUj and dTj/dUi contributions
  // excludes contributions to node i and j, which were considered as local contributions
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    // calculation of jacobian contribution to current_node's neighbors
    std::vector<dof_id_type> ivardofs(_nnodes);
    ivardofs[0] = _current_elem->node_ptr(nd)->dof_number(_sys.number(), _var.number(), 0);
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

    dof_id_type nb_index =
        std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - nd)) -
        neighbors.begin();
    std::vector<dof_id_type> dg_neighbors =
        _pdmesh.getBondDeformationGradientNeighbors(_current_elem->node_id(nd), nb_index);

    Real vol_nb;
    RealGradient origin_vec_nb;
    RankTwoTensor dFdUk, dPxdUkx;

    for (unsigned int nb = 0; nb < dg_neighbors.size(); ++nb)
      if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[nb]])) > 0.5)
      {
        ivardofs[1] = _pdmesh.nodePtr(neighbors[dg_neighbors[nb]])
                          ->dof_number(_sys.number(), _var.number(), 0);
        vol_nb = _pdmesh.getNodeVolume(neighbors[dg_neighbors[nb]]);

        origin_vec_nb = _pdmesh.getNodeCoord(neighbors[dg_neighbors[nb]]) -
                        _pdmesh.getNodeCoord(_current_elem->node_id(nd));

        dFdUk.zero();
        for (unsigned int i = 0; i < _dim; ++i)
          dFdUk(_component, i) =
              _horizon_radius[nd] / origin_vec_nb.norm() * origin_vec_nb(i) * vol_nb;

        dFdUk *= _shape2[nd].inverse();

        dPxdUkx = _Jacobian_mult[nd] * 0.5 * (dFdUk.transpose() + dFdUk);

        for (unsigned int i = 0; i < _nnodes; ++i)
          for (unsigned int j = 0; j < _nnodes; ++j)
            _local_ke(i, j) = (i == 0 ? -1 : 1) * (j == 0 ? 0 : 1) * _multi[nd] *
                              (dPxdUkx * _shape2[nd].inverse()).row(_component) * _origin_vec *
                              _bond_status;

        addJacobian(_assembly, _local_ke, _ivardofs, ivardofs, _var.scalingFactor());

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
            diag_save_in_dofs[0] = _current_elem->node_ptr(nd)->dof_number(
                _diag_save_in[i]->sys().number(), _diag_save_in[i]->number(), 0);
            diag_save_in_dofs[1] =
                _pdmesh.nodePtr(neighbors[dg_neighbors[nb]])
                    ->dof_number(_diag_save_in[i]->sys().number(), _diag_save_in[i]->number(), 0);

            _diag_save_in[i]->sys().solution().add_vector(diag, diag_save_in_dofs);
          }
        }
      }
  }
}

void
HorizonStabilizedFormISmallStrainMechanicsNOSPD::computeLocalOffDiagJacobian(
    unsigned int jvar_num, unsigned int coupled_component)
{
  _local_ke.zero();
  if (_temp_coupled && jvar_num == _temp_var->number()) // temperature is coupled
  {
    std::vector<RankTwoTensor> dSdT(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
      for (unsigned int es = 0; es < _deigenstrain_dT.size(); ++es)
        dSdT[nd] = -_Jacobian_mult[nd] * (*_deigenstrain_dT[es])[nd];

    for (unsigned int i = 0; i < _nnodes; ++i)
      for (unsigned int j = 0; j < _nnodes; ++j)
        _local_ke(i, j) += (i == 0 ? -1 : 1) * _multi[j] *
                           (dSdT[j] * _shape2[j].inverse()).row(_component) * _origin_vec *
                           _bond_status;
  }
  else if (_out_of_plane_strain_coupled &&
           jvar_num == _out_of_plane_strain_var
                           ->number()) // weak plane stress case, out_of_plane_strain is coupled
  {
    std::vector<RankTwoTensor> dSdE33(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
      for (unsigned int i = 0; i < 3; ++i)
        for (unsigned int j = 0; j < 3; ++j)
          dSdE33[nd](i, j) = _Jacobian_mult[nd](i, j, 2, 2);

    for (unsigned int i = 0; i < _nnodes; ++i)
      for (unsigned int j = 0; j < _nnodes; ++j)
        _local_ke(i, j) += (i == 0 ? -1 : 1) * _multi[j] *
                           (dSdE33[j] * _shape2[j].inverse()).row(_component) * _origin_vec *
                           _bond_status;
  }
  else // off-diagonal Jacobian with respect to other displacement variables
  {
    // ONLY consider the contributions to node i and j
    // contributions to their neighbors are considered as nonlocal off-diagonal
    for (unsigned int i = 0; i < _nnodes; ++i)
      for (unsigned int j = 0; j < _nnodes; ++j)
        _local_ke(i, j) +=
            (i == 0 ? -1 : 1) * _multi[j] *
            (computeDSDU(coupled_component, j) * _shape2[j].inverse()).row(_component) *
            _origin_vec * _bond_status;
  }
}

void
HorizonStabilizedFormISmallStrainMechanicsNOSPD::computePDNonlocalOffDiagJacobian(
    unsigned int jvar_num, unsigned int coupled_component)
{
  if (_temp_coupled && jvar_num == _temp_var->number())
  {
    // no nonlocal contribution from temperature
  }
  else if (_out_of_plane_strain_coupled && jvar_num == _out_of_plane_strain_var->number())
  {
    // no nonlocal contribution from out of plane strain
  }
  else
  {
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
    {
      // calculation of jacobian contribution to current_node's neighbors
      // NOT including the contribution to nodes i and j, which is considered as local off-diagonal
      std::vector<dof_id_type> jvardofs(_nnodes);
      jvardofs[0] = _current_elem->node_ptr(nd)->dof_number(_sys.number(), jvar_num, 0);
      std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
      std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

      dof_id_type nb_index =
          std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - nd)) -
          neighbors.begin();
      std::vector<dof_id_type> dg_neighbors =
          _pdmesh.getBondDeformationGradientNeighbors(_current_elem->node_id(nd), nb_index);

      Real vol_nb;
      RealGradient origin_vec_nb;
      RankTwoTensor dFdUk, dPxdUky;

      for (unsigned int nb = 0; nb < dg_neighbors.size(); ++nb)
        if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[nb]])) > 0.5)
        {
          jvardofs[1] =
              _pdmesh.nodePtr(neighbors[dg_neighbors[nb]])->dof_number(_sys.number(), jvar_num, 0);
          vol_nb = _pdmesh.getNodeVolume(neighbors[dg_neighbors[nb]]);

          origin_vec_nb = _pdmesh.getNodeCoord(neighbors[dg_neighbors[nb]]) -
                          _pdmesh.getNodeCoord(_current_elem->node_id(nd));

          dFdUk.zero();
          for (unsigned int i = 0; i < _dim; ++i)
            dFdUk(coupled_component, i) =
                _horizon_radius[nd] / origin_vec_nb.norm() * origin_vec_nb(i) * vol_nb;

          dFdUk *= _shape2[nd].inverse();

          dPxdUky = _Jacobian_mult[nd] * 0.5 * (dFdUk.transpose() + dFdUk);

          for (unsigned int i = 0; i < _nnodes; ++i)
            for (unsigned int j = 0; j < _nnodes; ++j)
              _local_ke(i, j) = (i == 0 ? -1 : 1) * (j == 0 ? 0 : 1) * _multi[nd] *
                                (dPxdUky * _shape2[nd].inverse()).row(_component) * _origin_vec *
                                _bond_status;

          addJacobian(_assembly, _local_ke, _ivardofs, jvardofs, _var.scalingFactor());
        }
    }
  }
}
