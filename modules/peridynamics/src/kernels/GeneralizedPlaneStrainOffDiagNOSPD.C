//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedPlaneStrainOffDiagNOSPD.h"
#include "MooseVariableScalar.h"
#include "PeridynamicsMesh.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("PeridynamicsApp", GeneralizedPlaneStrainOffDiagNOSPD);

template <>
InputParameters
validParams<GeneralizedPlaneStrainOffDiagNOSPD>()
{
  InputParameters params = validParams<MechanicsBaseNOSPD>();
  params.addClassDescription(
      "Class for calculating off-diagonal Jacobian corresponding to "
      "coupling between displacements (or temperature) with scalar out-of-plane strain for "
      "generalized plane strain using SNOSPD formulation");

  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for strain in the out-of-plane direction");

  return params;
}

GeneralizedPlaneStrainOffDiagNOSPD::GeneralizedPlaneStrainOffDiagNOSPD(
    const InputParameters & parameters)
  : MechanicsBaseNOSPD(parameters),
    _scalar_out_of_plane_strain_var_num(coupledScalar("scalar_out_of_plane_strain"))
{
  // Consistency check
  if (_disp_var.size() != 2)
    mooseError("GeneralizedPlaneStrain only works for two dimensional models!");
}

void
GeneralizedPlaneStrainOffDiagNOSPD::computeOffDiagJacobianScalar(unsigned int jvar_num)
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
GeneralizedPlaneStrainOffDiagNOSPD::computeDispPartialOffDiagJacobianScalar(unsigned int component,
                                                                            unsigned int jvar_num)
{
  // off-diagonal jacobian entries on the column and row corresponding to
  // scalar_out_of_plane_strain for coupling with displacements

  DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), jvar_num);
  DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar_num, _var.number());
  MooseVariableScalar & jvar = _sys.getScalarVariable(_tid, jvar_num);

  // fill in the column corresponding to the scalar variable
  std::vector<RankTwoTensor> dSdE33(_nnodes);
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
        dSdE33[nd](i, j) = _Jacobian_mult[nd](i, j, 2, 2);

  for (_i = 0; _i < _test.size(); ++_i)
    for (_j = 0; _j < jvar.order(); ++_j)
      ken(_i, _j) += (_i == _j ? -1 : 1) *
                     (_multi[0] * (dSdE33[0] * _shape[0].inverse()).row(component) +
                      _multi[1] * (dSdE33[1] * _shape[1].inverse()).row(component)) *
                     _origin_vec_ij * _bond_status_ij;

  kne(0, 0) += computeDSDU(component, 0)(2, 2) * _vols_ij[0] * _dg_vol_frac_ij[0] *
               _bond_status_ij; // node i
  kne(0, 1) += computeDSDU(component, 1)(2, 2) * _vols_ij[1] * _dg_vol_frac_ij[1] *
               _bond_status_ij; // node j
}

void
GeneralizedPlaneStrainOffDiagNOSPD::computeDispFullOffDiagJacobianScalar(unsigned int component,
                                                                         unsigned int jvar_num)
{
  // LOCAL contribution

  // off-diagonal jacobian entries on the column and row corresponding to
  // scalar_out_of_plane_strain for coupling with displacements
  DenseMatrix<Number> & ken = _assembly.jacobianBlock(_var.number(), jvar_num);
  DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar_num, _var.number());
  MooseVariableScalar & jvar = _sys.getScalarVariable(_tid, jvar_num);

  // fill in the column corresponding to the scalar variable
  std::vector<RankTwoTensor> dSdE33(_nnodes);
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
        dSdE33[nd](i, j) = _Jacobian_mult[nd](i, j, 2, 2);

  for (_i = 0; _i < _test.size(); ++_i)
    for (_j = 0; _j < jvar.order(); ++_j)
      ken(_i, _j) += (_i == _j ? -1 : 1) *
                     (_multi[0] * (dSdE33[0] * _shape[0].inverse()).row(component) +
                      _multi[1] * (dSdE33[1] * _shape[1].inverse()).row(component)) *
                     _origin_vec_ij * _bond_status_ij;

  // fill in the row corresponding to the scalar variable
  kne(0, 0) += computeDSDU(component, 0)(2, 2) * _vols_ij[0] * _dg_vol_frac_ij[0] *
               _bond_status_ij; // node i
  kne(0, 1) += computeDSDU(component, 1)(2, 2) * _vols_ij[1] * _dg_vol_frac_ij[1] *
               _bond_status_ij; // node j

  // NONLOCAL contribution

  // fill in the row corresponding to the scalar variable
  for (unsigned int cur_nd = 0; cur_nd < _nnodes; ++cur_nd)
  {
    // calculation of jacobian contribution to current_node's neighbors
    // NOT including the contribution to nodes i and j, which is considered as local off-diagonal
    std::vector<dof_id_type> ivardofs(_nnodes);
    ivardofs[0] = _current_elem->node_ptr(cur_nd)->dof_number(_sys.number(), _var.number(), 0);
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(cur_nd));
    unsigned int nb =
        std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - cur_nd)) -
        neighbors.begin();
    std::vector<unsigned int> dg_neighbors =
        _pdmesh.getDefGradNeighbors(_current_elem->node_id(cur_nd), nb);
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(cur_nd));
    for (unsigned int k = 0; k < dg_neighbors.size(); ++k)
    {
      Node * node_k = _pdmesh.nodePtr(neighbors[dg_neighbors[k]]);
      ivardofs[1] = node_k->dof_number(_sys.number(), _var.number(), 0);
      Real vol_k = _pdmesh.getPDNodeVolume(neighbors[dg_neighbors[k]]);

      // obtain bond k's origin vector
      RealGradient origin_vec_ijk = *node_k - *_pdmesh.nodePtr(_current_elem->node_id(cur_nd));

      RankTwoTensor dFdUk;
      dFdUk.zero();
      for (unsigned int j = 0; j < _dim; ++j)
        dFdUk(component, j) =
            _horiz_rad[cur_nd] / origin_vec_ijk.norm() * origin_vec_ijk(j) * vol_k;

      dFdUk *= _shape[cur_nd].inverse();

      RankTwoTensor dPdUk =
          _Jacobian_mult[cur_nd] * 0.5 *
          (dFdUk.transpose() * _dgrad[cur_nd] + _dgrad[cur_nd].transpose() * dFdUk);

      // bond status for bond k
      Real bond_status_ijk =
          _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[k]]));

      _local_ke.resize(ken.n(), ken.m());
      _local_ke.zero();
      _local_ke(0, 1) = dPdUk(2, 2) * _dg_vol_frac_ij[cur_nd] * _vols_ij[cur_nd] * _bond_status_ij *
                        bond_status_ijk;

      _assembly.cacheJacobianBlock(_local_ke, jvar.dofIndices(), ivardofs, _var.scalingFactor());
    }
  }
}

void
GeneralizedPlaneStrainOffDiagNOSPD::computeTempOffDiagJacobianScalar(unsigned int jvar_num)
{
  // off-diagonal jacobian entries on the row corresponding to scalar_out_of_plane_strain for
  // coupling with temperature
  DenseMatrix<Number> & kne = _assembly.jacobianBlock(jvar_num, _var.number());

  // one-way coupling between the scalar_out_of_plane_strain and temperature. fill in the row
  // corresponding to the scalar_out_of_plane_strain
  std::vector<RankTwoTensor> dSdT(_nnodes);
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
    for (unsigned int es = 0; es < _deigenstrain_dT.size(); ++es)
      dSdT[nd] = -_Jacobian_mult[nd] * (*_deigenstrain_dT[es])[nd];

  kne(0, 0) += dSdT[0](2, 2) * _dg_vol_frac_ij[0] * _vols_ij[0] * _bond_status_ij; // node i
  kne(0, 1) += dSdT[1](2, 2) * _dg_vol_frac_ij[1] * _vols_ij[1] * _bond_status_ij; // node j
}
