//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ForceStabilizedSmallStrainNOSPD.h"
#include "RankFourTensor.h"

registerMooseObject("PeridynamicsApp", ForceStabilizedSmallStrainNOSPD);

template <>
InputParameters
validParams<ForceStabilizedSmallStrainNOSPD>()
{
  InputParameters params = validParams<SmallStrainNOSPD>();
  params.addClassDescription(
      "Class for computing bond interaction for force-stabilized peridynamic correspondence model");

  return params;
}

ForceStabilizedSmallStrainNOSPD::ForceStabilizedSmallStrainNOSPD(const InputParameters & parameters)
  : SmallStrainNOSPD(parameters),
    _Cijkl(getMaterialProperty<RankFourTensor>("elasticity_tensor")),
    _sf_coeff(declareProperty<Real>("stabilization_force_coeff"))
{
}

void
ForceStabilizedSmallStrainNOSPD::computeQpDeformationGradient()
{
  _shape_tensor[_qp].zero();
  _deformation_gradient[_qp].zero();
  _ddgraddu[_qp].zero();
  _ddgraddv[_qp].zero();
  _ddgraddw[_qp].zero();

  if (_dim == 2)
    _shape_tensor[_qp](2, 2) = _deformation_gradient[_qp](2, 2) = 1.0;

  const Node * cur_nd = _current_elem->node_ptr(_qp);
  std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(cur_nd->id());
  std::vector<dof_id_type> bonds = _pdmesh.getAssocBonds(cur_nd->id());

  // calculate the shape tensor and prepare the deformation gradient tensor
  RealGradient ori_vec(_dim);
  RealGradient cur_vec(_dim);

  for (unsigned int j = 0; j < neighbors.size(); ++j)
    if (_bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[j])) > 0.5)
    {
      Node * node_j = _pdmesh.nodePtr(neighbors[j]);
      Real vol_j = _pdmesh.getVolume(neighbors[j]);
      ori_vec = *node_j - *_pdmesh.nodePtr(cur_nd->id());

      for (unsigned int k = 0; k < _dim; ++k)
        cur_vec(k) = ori_vec(k) + _disp_var[k]->getNodalValue(*node_j) -
                     _disp_var[k]->getNodalValue(*cur_nd);

      Real ori_len = ori_vec.norm();
      for (unsigned int k = 0; k < _dim; ++k)
      {
        for (unsigned int l = 0; l < _dim; ++l)
        {
          _shape_tensor[_qp](k, l) += vol_j * _horizon[_qp] / ori_len * ori_vec(k) * ori_vec(l);
          _deformation_gradient[_qp](k, l) +=
              vol_j * _horizon[_qp] / ori_len * cur_vec(k) * ori_vec(l);
        }
        // calculate derivatives of deformation_gradient w.r.t displacements of node i
        _ddgraddu[_qp](0, k) += -vol_j * _horizon[_qp] / ori_len * ori_vec(k);
        _ddgraddv[_qp](1, k) += -vol_j * _horizon[_qp] / ori_len * ori_vec(k);
        if (_dim == 3)
          _ddgraddw[_qp](2, k) += -vol_j * _horizon[_qp] / ori_len * ori_vec(k);
      }
    }

  // finalize the deformation gradient tensor
  _deformation_gradient[_qp] *= _shape_tensor[_qp].inverse();
  _ddgraddu[_qp] *= _shape_tensor[_qp].inverse();
  _ddgraddv[_qp] *= _shape_tensor[_qp].inverse();
  _ddgraddw[_qp] *= _shape_tensor[_qp].inverse();

  Real youngs_modulus = ElasticityTensorTools::getIsotropicYoungsModulus(_Cijkl[_qp]);
  _sf_coeff[_qp] = youngs_modulus * _pdmesh.getMeshSpacing(_current_elem->node_id(_qp)) *
                   _horizon[_qp] / _origin_length;
  _multi[_qp] = _horizon[_qp] / _origin_length * _nv[0] * _nv[1];
}
