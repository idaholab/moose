//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedPlaneStrainOffDiagOSPD.h"
#include "MooseVariableScalar.h"
#include "PeridynamicsMesh.h"

registerMooseObject("PeridynamicsApp", GeneralizedPlaneStrainOffDiagOSPD);

InputParameters
GeneralizedPlaneStrainOffDiagOSPD::validParams()
{
  InputParameters params = MechanicsBasePD::validParams();
  params.addClassDescription(
      "Class for calculating the off-diagonal Jacobian corresponding to "
      "coupling between displacements (or temperature) and the scalar out-of-plane strain for "
      "the generalized plane strain using the OSPD formulation");

  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for strain in the out-of-plane direction");

  return params;
}

GeneralizedPlaneStrainOffDiagOSPD::GeneralizedPlaneStrainOffDiagOSPD(
    const InputParameters & parameters)
  : MechanicsBasePD(parameters),
    _bond_local_dfdE(getMaterialProperty<Real>("bond_local_dfdE")),
    _bond_nonlocal_dfdE(getMaterialProperty<Real>("bond_nonlocal_dfdE")),
    _alpha(getMaterialProperty<Real>("thermal_expansion_coeff")),
    _Cijkl(getMaterialProperty<RankFourTensor>("elasticity_tensor")),
    _scalar_out_of_plane_strain_var_num(coupledScalar("scalar_out_of_plane_strain"))
{
  // Consistency check
  if (_disp_var.size() != 2)
    mooseError("GeneralizedPlaneStrain only works for two dimensional case!");
}

void
GeneralizedPlaneStrainOffDiagOSPD::computeOffDiagJacobianScalar(unsigned int jvar_num)
{
  if (jvar_num == _scalar_out_of_plane_strain_var_num)
  {
    prepare();

    if (_var.number() == _disp_var[0]->number())
      if (_use_full_jacobian)
        computeDispFullOffDiagJacobianScalar(0, jvar_num);
      else
        computeDispPartialOffDiagJacobianScalar(0, jvar_num);
    else if (_var.number() == _disp_var[1]->number())
      if (_use_full_jacobian)
        computeDispFullOffDiagJacobianScalar(1, jvar_num);
      else
        computeDispPartialOffDiagJacobianScalar(1, jvar_num);
    else if (_temp_coupled ? _var.number() == _temp_var->number() : 0)
      computeTempOffDiagJacobianScalar(jvar_num);
  }
}

void
GeneralizedPlaneStrainOffDiagOSPD::computeDispFullOffDiagJacobianScalar(unsigned int component,
                                                                        unsigned int jvar_num)
{
  prepareMatrixTag(_assembly, _var.number(), jvar_num, _ken);
  prepareMatrixTag(_assembly, jvar_num, _var.number(), _kne);
  MooseVariableScalar & jvar = _sys.getScalarVariable(_tid, jvar_num);

  // LOCAL jacobian contribution
  // fill in the column corresponding to the scalar variable from bond ij
  for (unsigned int i = 0; i < _nnodes; ++i)
    for (unsigned int j = 0; j < jvar.order(); ++j)
      _ken(i, j) +=
          (i == j ? -1 : 1) * _current_unit_vec(component) * _bond_local_dfdE[0] * _bond_status;

  // NONLOCAL jacobian contribution
  std::vector<RankTwoTensor> shape(_nnodes), dgrad(_nnodes);

  // for off-diagonal low components
  if (_bond_status > 0.5)
  {
    for (unsigned int nd = 0; nd < _nnodes; nd++)
    {
      if (_dim == 2)
        shape[nd](2, 2) = dgrad[nd](2, 2) = 1.0;
      // calculation of jacobian contribution to current node's neighbors
      std::vector<dof_id_type> ivardofs(_nnodes);
      ivardofs[nd] = _ivardofs[nd];
      std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
      std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

      Real vol_nb, weight_nb;
      RealGradient origin_vec_nb, current_vec_nb;

      for (unsigned int nb = 0; nb < neighbors.size(); nb++)
        if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)
        {
          ivardofs[1 - nd] =
              _pdmesh.nodePtr(neighbors[nb])->dof_number(_sys.number(), _var.number(), 0);
          vol_nb = _pdmesh.getNodeVolume(neighbors[nb]);

          // obtain bond nb's origin length and current orientation
          origin_vec_nb =
              *_pdmesh.nodePtr(neighbors[nb]) - *_pdmesh.nodePtr(_current_elem->node_id(nd));

          for (unsigned int k = 0; k < _dim; k++)
            current_vec_nb(k) = origin_vec_nb(k) +
                                _disp_var[k]->getNodalValue(*_pdmesh.nodePtr(neighbors[nb])) -
                                _disp_var[k]->getNodalValue(*_current_elem->node_ptr(nd));

          weight_nb = _horizon_radius[nd] / origin_vec_nb.norm();
          // prepare shape tensor and deformation gradient tensor for current node
          for (unsigned int k = 0; k < _dim; k++)
            for (unsigned int l = 0; l < _dim; l++)
            {
              shape[nd](k, l) += weight_nb * origin_vec_nb(k) * origin_vec_nb(l) * vol_nb;
              dgrad[nd](k, l) += weight_nb * current_vec_nb(k) * origin_vec_nb(l) * vol_nb;
            }

          // cache the nonlocal jacobian contribution
          _local_ke.resize(_ken.m(), _ken.n());
          _local_ke.zero();
          _local_ke(0, 0) = (nd == 0 ? -1 : 1) * current_vec_nb(component) / current_vec_nb.norm() *
                            _bond_nonlocal_dfdE[nd] / origin_vec_nb.norm() * vol_nb * _bond_status;
          _local_ke(1, 0) = (nd == 0 ? 1 : -1) * current_vec_nb(component) / current_vec_nb.norm() *
                            _bond_nonlocal_dfdE[nd] / origin_vec_nb.norm() * vol_nb * _bond_status;

          addJacobian(_assembly, _local_ke, ivardofs, jvar.dofIndices(), _var.scalingFactor());
        }

      // finalize the shape tensor and deformation gradient tensor for node_i
      if (shape[nd].det() == 0.)
        mooseError("Singular shape tensor is detected in GeneralizedPlaneStrainOffDiagOSPD! Use "
                   "SingularShapeTensorEliminatorUserObjectPD to avoid singular shape tensor");

      shape[nd] = shape[nd].inverse();
      dgrad[nd] = dgrad[nd] * shape[nd];
    }
  }

  // off-diagonal jacobian entries on the row
  Real dEidUi =
      -_node_vol[1] * _horizon_radius[0] / _origin_vec.norm() *
      (_Cijkl[0](2, 2, 0, 0) * (_origin_vec(0) * shape[0](0, 0) + _origin_vec(1) * shape[0](1, 0)) *
           dgrad[0](component, 0) +
       _Cijkl[0](2, 2, 1, 1) * (_origin_vec(0) * shape[0](0, 1) + _origin_vec(1) * shape[0](1, 1)) *
           dgrad[0](component, 1));
  Real dEjdUj =
      _node_vol[0] * _horizon_radius[1] / _origin_vec.norm() *
      (_Cijkl[0](2, 2, 0, 0) * (_origin_vec(0) * shape[1](0, 0) + _origin_vec(1) * shape[1](1, 0)) *
           dgrad[1](component, 0) +
       _Cijkl[0](2, 2, 1, 1) * (_origin_vec(0) * shape[1](0, 1) + _origin_vec(1) * shape[1](1, 1)) *
           dgrad[1](component, 1));

  // fill in the row corresponding to the scalar variable
  _kne(0, 0) += (dEidUi * _node_vol[0] - dEjdUj * _node_vol[1]) * _bond_status; // node i
  _kne(0, 1) += (dEjdUj * _node_vol[1] - dEidUi * _node_vol[0]) * _bond_status; // node j

  accumulateTaggedLocalMatrix(_assembly, _var.number(), jvar_num, _ken);
  accumulateTaggedLocalMatrix(_assembly, jvar_num, _var.number(), _kne);
}

void
GeneralizedPlaneStrainOffDiagOSPD::computeDispPartialOffDiagJacobianScalar(unsigned int component,
                                                                           unsigned int jvar_num)
{
  prepareMatrixTag(_assembly, _var.number(), jvar_num, _ken);
  prepareMatrixTag(_assembly, jvar_num, _var.number(), _kne);
  MooseVariableScalar & jvar = _sys.getScalarVariable(_tid, jvar_num);

  // fill in the column corresponding to the scalar variable from bond ij
  for (unsigned int i = 0; i < _nnodes; ++i)
    for (unsigned int j = 0; j < jvar.order(); ++j)
    {
      _ken(i, j) +=
          (i == j ? -1 : 1) * _current_unit_vec(component) * _bond_local_dfdE[0] * _bond_status;
      _kne(j, i) +=
          (i == j ? -1 : 1) * _current_unit_vec(component) * _bond_local_dfdE[0] * _bond_status;
    }

  accumulateTaggedLocalMatrix(_assembly, _var.number(), jvar_num, _ken);
  accumulateTaggedLocalMatrix(_assembly, jvar_num, _var.number(), _kne);
}

void
GeneralizedPlaneStrainOffDiagOSPD::computeTempOffDiagJacobianScalar(unsigned int jvar_num)
{
  // off-diagonal jacobian entries on the row
  prepareMatrixTag(_assembly, jvar_num, _var.number(), _kne);

  // calculate number of active neighbors for node i and j
  std::vector<unsigned int> active_neighbors(_nnodes, 0);
  for (unsigned int nd = 0; nd < _nnodes; nd++)
  {
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));
    for (unsigned int nb = 0; nb < bonds.size(); ++nb)
      if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)
        active_neighbors[nd]++;

    if (active_neighbors[nd] == 0) // avoid dividing by zero
      active_neighbors[nd] = 1;
  }

  //  one-way coupling between the strain_zz and temperature. fill in the row corresponding to the
  //  scalar variable strain_zz
  _kne(0, 0) += -_alpha[0] *
                (_Cijkl[0](2, 2, 0, 0) + _Cijkl[0](2, 2, 1, 1) + _Cijkl[0](2, 2, 2, 2)) *
                _node_vol[0] / active_neighbors[0] * _bond_status; // node i
  _kne(0, 1) += -_alpha[0] *
                (_Cijkl[0](2, 2, 0, 0) + _Cijkl[0](2, 2, 1, 1) + _Cijkl[0](2, 2, 2, 2)) *
                _node_vol[1] / active_neighbors[1] * _bond_status; // node j

  accumulateTaggedLocalMatrix(_assembly, jvar_num, _var.number(), _kne);
}
