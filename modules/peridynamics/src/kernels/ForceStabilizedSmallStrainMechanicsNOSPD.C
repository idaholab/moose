//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ForceStabilizedSmallStrainMechanicsNOSPD.h"
#include "PeridynamicsMesh.h"

registerMooseObject("PeridynamicsApp", ForceStabilizedSmallStrainMechanicsNOSPD);

InputParameters
ForceStabilizedSmallStrainMechanicsNOSPD::validParams()
{
  InputParameters params = MechanicsBaseNOSPD::validParams();
  params.addClassDescription(
      "Class for calculating the residual and Jacobian for the force-stabilized peridynamic "
      "correspondence model under small strain assumptions");

  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the variable this kernel acts on (0 for x, 1 for y, 2 for z)");

  return params;
}

ForceStabilizedSmallStrainMechanicsNOSPD::ForceStabilizedSmallStrainMechanicsNOSPD(
    const InputParameters & parameters)
  : MechanicsBaseNOSPD(parameters),
    _sf_coeff(getMaterialProperty<Real>("stabilization_force_coeff")),
    _component(getParam<unsigned int>("component"))
{
}

void
ForceStabilizedSmallStrainMechanicsNOSPD::computeLocalResidual()
{
  Real sforce = 0.5 * (_sf_coeff[0] + _sf_coeff[1]) *
                (_disp_var[_component]->getNodalValue(*_current_elem->node_ptr(1)) -
                 _disp_var[_component]->getNodalValue(*_current_elem->node_ptr(0))) *
                _bond_status;

  // For small strain assumptions, stress measures, i.e., Cauchy stress (Sigma), the first
  // Piola-Kirchhoff stress (P), and the second Piola-Kirchhoff stress (S) are approximately the
  // same. Thus, the nodal force state tensors are calculated using the Cauchy stresses,
  // i.e., T = Sigma * inv(Shape) * xi * multi.
  // Cauchy stress is calculated as Sigma = C * E.

  for (unsigned int nd = 0; nd < _nnodes; ++nd)
    for (unsigned int i = 0; i < _nnodes; ++i)
      _local_re(i) += (i == 0 ? -1 : 1) * _multi[nd] *
                      (_stress[nd] * _shape2[nd].inverse()).row(_component) * _origin_vec *
                      _bond_status;

  // add fictitious force for model stabilization
  _local_re(0) += -sforce;
  _local_re(1) += sforce;
}

void
ForceStabilizedSmallStrainMechanicsNOSPD::computeLocalJacobian()
{
  // excludes dTi/dUj and dTj/dUi contribution which was considered as nonlocal contribution
  for (unsigned int i = 0; i < _nnodes; ++i)
    for (unsigned int j = 0; j < _nnodes; ++j)
      _local_ke(i, j) += (i == 0 ? -1 : 1) * _multi[j] *
                         (computeDSDU(_component, j) * _shape2[j].inverse()).row(_component) *
                         _origin_vec * _bond_status;

  // compute local stabilization force jacobian
  Real diag = 0.5 * (_sf_coeff[0] + _sf_coeff[1]);
  for (unsigned int i = 0; i < _nnodes; ++i)
    for (unsigned int j = 0; j < _nnodes; ++j)
      _local_ke(i, j) += (i == j ? 1 : -1) * diag * _bond_status;
}

void
ForceStabilizedSmallStrainMechanicsNOSPD::computeNonlocalJacobian()
{
  // includes dTi/dUj and dTj/dUi contributions
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    // calculation of jacobian contribution to current_node's neighbors
    std::vector<dof_id_type> ivardofs(_nnodes);
    ivardofs[0] = _current_elem->node_ptr(nd)->dof_number(_sys.number(), _var.number(), 0);
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

    Real vol_nb;
    RealGradient origin_vec_nb;
    RankTwoTensor dFdUk, dSxdUkx;

    for (unsigned int nb = 0; nb < neighbors.size(); ++nb)
      if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)
      {
        Node * neighbor_nb = _pdmesh.nodePtr(neighbors[nb]);
        ivardofs[1] = neighbor_nb->dof_number(_sys.number(), _var.number(), 0);
        vol_nb = _pdmesh.getNodeVolume(neighbors[nb]);

        // obtain bond ndnb's origin vector
        origin_vec_nb = *neighbor_nb - *_pdmesh.nodePtr(_current_elem->node_id(nd));

        dFdUk.zero();
        for (unsigned int i = 0; i < _dim; ++i)
          dFdUk(_component, i) =
              vol_nb * _horizon_radius[nd] / origin_vec_nb.norm() * origin_vec_nb(i);

        dFdUk *= _shape2[nd].inverse();
        dSxdUkx = _Jacobian_mult[nd] * 0.5 * (dFdUk.transpose() + dFdUk);

        _local_ke.zero();
        for (unsigned int i = 0; i < _nnodes; ++i)
          for (unsigned int j = 0; j < _nnodes; ++j)
            _local_ke(i, j) = (i == 0 ? -1 : 1) * (j == 0 ? 0 : 1) * _multi[nd] *
                              (dSxdUkx * _shape2[nd].inverse()).row(_component) * _origin_vec *
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
            _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
        }
      }
  }
}

void
ForceStabilizedSmallStrainMechanicsNOSPD::computeLocalOffDiagJacobian(
    unsigned int jvar_num, unsigned int coupled_component)
{
  _local_ke.zero();
  if (_temp_coupled && jvar_num == _temp_var->number())
  {
    std::vector<RankTwoTensor> dSdT(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
      for (unsigned int es = 0; es < _deigenstrain_dT.size(); ++es)
        dSdT[nd] = -(_Jacobian_mult[nd] * (*_deigenstrain_dT[es])[nd]);

    for (unsigned int i = 0; i < _nnodes; ++i)
      for (unsigned int j = 0; j < _nnodes; ++j)
        _local_ke(i, j) += (i == 0 ? -1 : 1) * _multi[j] *
                           (dSdT[j] * _shape2[j].inverse()).row(_component) * _origin_vec *
                           _bond_status;
  }
  else
  {
    for (unsigned int i = 0; i < _nnodes; ++i)
      for (unsigned int j = 0; j < _nnodes; ++j)
        _local_ke(i, j) +=
            (i == 0 ? -1 : 1) * _multi[j] *
            (computeDSDU(coupled_component, j) * _shape2[j].inverse()).row(_component) *
            _origin_vec * _bond_status;
  }
}

void
ForceStabilizedSmallStrainMechanicsNOSPD::computePDNonlocalOffDiagJacobian(
    unsigned int jvar_num, unsigned int coupled_component)
{
  if (_temp_coupled && jvar_num == _temp_var->number())
  {
    // no nonlocal contribution from temperature
  }
  else
  {
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
    {
      // calculation of jacobian contribution to current_node's neighbors
      std::vector<dof_id_type> jvardofs(_nnodes);
      jvardofs[0] = _current_elem->node_ptr(nd)->dof_number(_sys.number(), jvar_num, 0);
      std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
      std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

      Real vol_nb;
      RealGradient origin_vec_nb;
      RankTwoTensor dFdUk, dSxdUky;

      for (unsigned int nb = 0; nb < neighbors.size(); ++nb)
        if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)

        {
          Node * neighbor_nb = _pdmesh.nodePtr(neighbors[nb]);
          jvardofs[1] = neighbor_nb->dof_number(_sys.number(), jvar_num, 0);
          vol_nb = _pdmesh.getNodeVolume(neighbors[nb]);

          // obtain bond ndnb's origin vector
          origin_vec_nb = *neighbor_nb - *_pdmesh.nodePtr(_current_elem->node_id(nd));

          dFdUk.zero();
          for (unsigned int i = 0; i < _dim; ++i)
            dFdUk(coupled_component, i) =
                _horizon_radius[nd] / origin_vec_nb.norm() * origin_vec_nb(i) * vol_nb;

          dFdUk *= _shape2[nd].inverse();
          dSxdUky = _Jacobian_mult[nd] * 0.5 * (dFdUk.transpose() + dFdUk);

          _local_ke.zero();
          for (unsigned int i = 0; i < _nnodes; ++i)
            for (unsigned int j = 0; j < _nnodes; ++j)
              _local_ke(i, j) = (i == 0 ? -1 : 1) * (j == 0 ? 0 : 1) * _multi[nd] *
                                (dSxdUky * _shape2[nd].inverse()).row(_component) * _origin_vec *
                                _bond_status;

          addJacobian(_assembly, _local_ke, _ivardofs, jvardofs, _var.scalingFactor());
        }
    }
  }
}
