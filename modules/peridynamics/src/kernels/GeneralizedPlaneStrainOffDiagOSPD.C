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
#include "MeshBasePD.h"
#include "RankTwoTensor.h"

registerMooseObject("PeridynamicsApp", GeneralizedPlaneStrainOffDiagOSPD);

template <>
InputParameters
validParams<GeneralizedPlaneStrainOffDiagOSPD>()
{
  InputParameters params = validParams<MechanicsBasePD>();
  params.addClassDescription(
      "Class for calculating off-diagonal Jacobian corresponding to "
      "coupling between displacements (or temperature) and scalar out-of-plane strain for "
      "generalized plane strain using OSPD formulation");
  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for strain in the out-of-plane direction");

  return params;
}

GeneralizedPlaneStrainOffDiagOSPD::GeneralizedPlaneStrainOffDiagOSPD(
    const InputParameters & parameters)
  : MechanicsBasePD(parameters),
    _bond_dfdE_ij(getMaterialProperty<Real>("bond_dfdE_ij")),
    _bond_dfdE_i_j(getMaterialProperty<Real>("bond_dfdE_i_j")),
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

  DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), jvar_num);
  DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar_num, _var.number());
  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar_num);

  // LOCAL jacobian contribution
  // fill in the column corresponding to the scalar variable from bond ij
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jv.order(); _j++)
      ken(_i, _j) +=
          (_i == _j ? -1 : 1) * _cur_ori_ij(component) * _bond_dfdE_ij[0] * _bond_status_ij;

  // NONLOCAL jacobian contribution
  std::vector<dof_id_type> dof(2), neighbors(2), bonds(2);
  // for off-diagonal low components
  RankTwoTensor delta(RankTwoTensor::initIdentity);
  std::vector<RankTwoTensor> shape(2), dgrad(2);

  for (unsigned int cur_nd = 0; cur_nd < 2; cur_nd++)
  {
    if (_dim == 2)
      shape[cur_nd](2, 2) = dgrad[cur_nd](2, 2) = 1.0;

    // calculation of jacobian contribution to current node's neighbors
    dof[cur_nd] = _ivardofs_ij[cur_nd];
    neighbors = _pdmesh.neighbors(_nodes_ij[cur_nd]->id());
    bonds = _pdmesh.bonds(_nodes_ij[cur_nd]->id());
    for (unsigned int k = 0; k < neighbors.size(); k++)
    {
      Node * node_k = _pdmesh.nodePtr(neighbors[k]);
      dof[1 - cur_nd] = node_k->dof_number(_sys.number(), _var.number(), 0);
      Real vol_k = _pdmesh.volume(neighbors[k]);

      // obtain bond k's origin length and current orientation
      RealGradient origin_ori_k(_dim), current_ori_k(_dim);
      for (unsigned int j = 0; j < _dim; j++)
      {
        origin_ori_k(j) =
            _pdmesh.coord(neighbors[k])(j) - _pdmesh.coord(_nodes_ij[cur_nd]->id())(j);
        current_ori_k(j) = origin_ori_k(j) + _disp_var[j]->getNodalValue(*node_k) -
                           _disp_var[j]->getNodalValue(*_nodes_ij[cur_nd]);
      }
      Real origin_len_k = origin_ori_k.norm();
      Real current_len_k = current_ori_k.norm();

      // bond status for bond k
      Real bond_status_k = _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[k]));

      // prepare shape tensor and deformation gradient tensor for current node
      for (unsigned int m = 0; m < _dim; m++)
        for (unsigned int n = 0; n < _dim; n++)
        {
          shape[cur_nd](m, n) += vol_k * _horizons_ij[cur_nd] / origin_len_k * origin_ori_k(m) *
                                 origin_ori_k(n) * bond_status_k;
          dgrad[cur_nd](m, n) += vol_k * _horizons_ij[cur_nd] / origin_len_k * current_ori_k(m) *
                                 origin_ori_k(n) * bond_status_k;
        }

      // cache the nonlocal jacobian contribution
      _local_ke.resize(ken.m(), ken.n());
      _local_ke.zero();
      _local_ke(0, 0) = (cur_nd == 0 ? -1 : 1) * current_ori_k(component) / current_len_k *
                        _bond_dfdE_i_j[cur_nd] / origin_len_k * vol_k * bond_status_k *
                        _bond_status_ij;
      _local_ke(1, 0) = (cur_nd == 0 ? 1 : -1) * current_ori_k(component) / current_len_k *
                        _bond_dfdE_i_j[cur_nd] / origin_len_k * vol_k * bond_status_k *
                        _bond_status_ij;

      _assembly.cacheJacobianBlock(_local_ke, dof, jv.dofIndices(), _var.scalingFactor());
    }

    // finalize the shape tensor and deformation gradient tensor for node_i
    if (MooseUtils::absoluteFuzzyEqual(shape[cur_nd].det(), 0.0))
    {
      shape[cur_nd] = delta;
      dgrad[cur_nd] = delta;
    }
    else
    {
      // inverse the shape tensor at node i
      shape[cur_nd] = shape[cur_nd].inverse();
      // calculate the deformation gradient tensor at node i
      dgrad[cur_nd] = dgrad[cur_nd] * shape[cur_nd];
    }
  }

  // off-diagonal jacobian entries on the row
  Real dEidUi = -_vols_ij[1] * _horizons_ij[0] / _origin_vec_ij.norm() *
                (_Cijkl[0](2, 2, 0, 0) *
                     (_origin_vec_ij(0) * shape[0](0, 0) + _origin_vec_ij(1) * shape[0](1, 0)) *
                     dgrad[0](component, 0) +
                 _Cijkl[0](2, 2, 1, 1) *
                     (_origin_vec_ij(0) * shape[0](0, 1) + _origin_vec_ij(1) * shape[0](1, 1)) *
                     dgrad[0](component, 1));
  Real dEjdUj = _vols_ij[0] * _horizons_ij[1] / _origin_vec_ij.norm() *
                (_Cijkl[0](2, 2, 0, 0) *
                     (_origin_vec_ij(0) * shape[1](0, 0) + _origin_vec_ij(1) * shape[1](1, 0)) *
                     dgrad[1](component, 0) +
                 _Cijkl[0](2, 2, 1, 1) *
                     (_origin_vec_ij(0) * shape[1](0, 1) + _origin_vec_ij(1) * shape[1](1, 1)) *
                     dgrad[1](component, 1));

  // fill in the row corresponding to the scalar variable
  kne(0, 0) += (dEidUi * _vols_ij[0] - dEjdUj * _vols_ij[1]) * _bond_status_ij; // node i
  kne(0, 1) += (dEjdUj * _vols_ij[1] - dEidUi * _vols_ij[0]) * _bond_status_ij; // node j
}

void
GeneralizedPlaneStrainOffDiagOSPD::computeDispPartialOffDiagJacobianScalar(unsigned int component,
                                                                           unsigned int jvar_num)
{
  DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), jvar_num);
  DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar_num, _var.number());
  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar_num);

  // fill in the column corresponding to the scalar variable from bond ij
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < jv.order(); _j++)
    {
      ken(_i, _j) +=
          (_i == _j ? -1 : 1) * _cur_ori_ij(component) * _bond_dfdE_ij[0] * _bond_status_ij;
      kne(_j, _i) +=
          (_i == _j ? -1 : 1) * _cur_ori_ij(component) * _bond_dfdE_ij[0] * _bond_status_ij;
    }
}

void
GeneralizedPlaneStrainOffDiagOSPD::computeTempOffDiagJacobianScalar(unsigned int jvar_num)
{
  // off-diagonal jacobian entries on the row

  DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar_num, _var.number());

  // number of neighbors for node i and j
  unsigned int nn_i = _pdmesh.neighbors(_nodes_ij[0]->id()).size();
  unsigned int nn_j = _pdmesh.neighbors(_nodes_ij[1]->id()).size();

  //  one-way coupling between the strain_zz and temperature. fill in the row corresponding to the
  //  scalar variable strain_zz
  kne(0, 0) += -_alpha[0] *
               (_Cijkl[0](2, 2, 0, 0) + _Cijkl[0](2, 2, 1, 1) + _Cijkl[0](2, 2, 2, 2)) *
               _vols_ij[0] / nn_i; // node i
  kne(0, 1) += -_alpha[0] *
               (_Cijkl[0](2, 2, 0, 0) + _Cijkl[0](2, 2, 1, 1) + _Cijkl[0](2, 2, 2, 2)) *
               _vols_ij[1] / nn_j; // node j
}
