//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialBaseNOSPD.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<MaterialBaseNOSPD>()
{
  InputParameters params = validParams<MechanicsMaterialBasePD>();
  params.addClassDescription(
      "Base class for Self-stabilized Non-Ordinary State-based PeriDynamic (SNOSPD) "
      "correspondence material model");

  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", "List of eigenstrains to be applied in this strain calculation");

  return params;
}

MaterialBaseNOSPD::MaterialBaseNOSPD(const InputParameters & parameters)
  : DerivativeMaterialInterface<MechanicsMaterialBasePD>(parameters),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _eigenstrains(_eigenstrain_names.size()),
    _shape_tensor(declareProperty<RankTwoTensor>("shape_tensor")),
    _deformation_gradient(declareProperty<RankTwoTensor>("deformation_gradient")),
    _ddgraddu(declareProperty<RankTwoTensor>("ddeformation_gradient_du")),
    _ddgraddv(declareProperty<RankTwoTensor>("ddeformation_gradient_dv")),
    _ddgraddw(declareProperty<RankTwoTensor>("ddeformation_gradient_dw")),
    _total_strain(declareProperty<RankTwoTensor>("total_strain")),
    _mechanical_strain(declareProperty<RankTwoTensor>("mechanical_strain")),
    _multi(declareProperty<Real>("multi"))
{
  for (unsigned int i = 0; i < _eigenstrains.size(); ++i)
  {
    _eigenstrain_names[i] = _eigenstrain_names[i];
    _eigenstrains[i] = &getMaterialProperty<RankTwoTensor>(_eigenstrain_names[i]);
  }
}

void
MaterialBaseNOSPD::initQpStatefulProperties()
{
  _mechanical_strain[_qp].zero();
  _total_strain[_qp].zero();
  _deformation_gradient[_qp].zero();
  _deformation_gradient[_qp].addIa(1.0);
  if (_qrule->n_points() < 2)
    mooseError("NOSPD models require a minimum quadrature order of THIRD");
}

void
MaterialBaseNOSPD::computeQpDeformationGradient()
{
  _shape_tensor[_qp].zero();
  _deformation_gradient[_qp].zero();
  _ddgraddu[_qp].zero();
  _ddgraddv[_qp].zero();
  _ddgraddw[_qp].zero();

  if (_dim == 2)
    _shape_tensor[_qp](2, 2) = _deformation_gradient[_qp](2, 2) = 1.0;

  const Node * cur_nd = _current_elem->node_ptr(_qp);
  const Node * end_nd = _current_elem->node_ptr(1 - _qp); // two nodes for edge2 element
  std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(cur_nd->id());
  std::vector<dof_id_type> bonds = _pdmesh.getAssocBonds(cur_nd->id());

  unsigned int nb = std::find(neighbors.begin(), neighbors.end(), end_nd->id()) - neighbors.begin();
  std::vector<unsigned int> BAneighbors = _pdmesh.getBondAssocHorizonNeighbors(cur_nd->id(), nb);

  if (BAneighbors.size() < _dim)
    mooseError("Not enough neighboring bonds for deformation gradient approximation for element: ",
               _current_elem->id(),
               ". Try to use larger bond-associated horizon!");

  // check the number of intact bonds for bond-associated deformation gradient calculation
  unsigned int intact_dg_bonds = 0;
  for (unsigned int j = 0; j < BAneighbors.size(); ++j)
    if (_bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[BAneighbors[j]])) > 0.5)
      ++intact_dg_bonds;
  // if there are no sufficient bonds to approximate the deformation gradient of current bond
  // expand the bond-associated horizon to the full horizon
  bool apply_bond_status = true;
  if (intact_dg_bonds < _dim)
    apply_bond_status = false;

  // calculate the shape tensor and prepare the deformation gradient tensor
  Real dgnodes_vsum = 0.0;
  RealGradient ori_vec(_dim), cur_vec(_dim);

  for (unsigned int j = 0; j < BAneighbors.size(); ++j)
    if (!apply_bond_status ||
        (apply_bond_status &&
         _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[BAneighbors[j]])) > 0.5))
    {
      Node * node_j = _pdmesh.nodePtr(neighbors[BAneighbors[j]]);
      Real vol_j = _pdmesh.getVolume(neighbors[BAneighbors[j]]);
      dgnodes_vsum += vol_j;
      ori_vec = *node_j - *_pdmesh.nodePtr(cur_nd->id());

      for (unsigned int k = 0; k < _dim; ++k)
        cur_vec(k) = ori_vec(k) + _disp_var[k]->getNodalValue(*node_j) -
                     _disp_var[k]->getNodalValue(*cur_nd);

      Real ori_len = ori_vec.norm();
      for (unsigned int k = 0; k < _dim; ++k)
      {
        for (unsigned int l = 0; l < _dim; ++l)
        {
          _shape_tensor[_qp](k, l) += _horizon[_qp] / ori_len * ori_vec(k) * ori_vec(l) * vol_j;
          _deformation_gradient[_qp](k, l) +=
              _horizon[_qp] / ori_len * cur_vec(k) * ori_vec(l) * vol_j;
        }
        // calculate derivatives of deformation_gradient w.r.t displacements of node i
        _ddgraddu[_qp](0, k) += -_horizon[_qp] / ori_len * ori_vec(k) * vol_j;
        _ddgraddv[_qp](1, k) += -_horizon[_qp] / ori_len * ori_vec(k) * vol_j;
        if (_dim == 3)
          _ddgraddw[_qp](2, k) += -_horizon[_qp] / ori_len * ori_vec(k) * vol_j;
      }
    }

  // force state multiplier
  _multi[_qp] = _horizon[_qp] / _origin_length * _nv[0] * _nv[1] * dgnodes_vsum / _nvsum[_qp];

  // finalize the deformation gradient and its derivatives
  // for cases when current bond was broken, assign shape tensor and deformation gradient to unity
  // and derivatives to zero
  if (_bond_status_var.getElementalValue(_current_elem) < 0.5)
  {
    _shape_tensor[_qp] = RankTwoTensor::initIdentity;
    _deformation_gradient[_qp] = RankTwoTensor::initIdentity;
    _ddgraddu[_qp].zero();
    _ddgraddv[_qp].zero();
    _ddgraddw[_qp].zero();
  }
  else
  {
    _deformation_gradient[_qp] *= _shape_tensor[_qp].inverse();
    _ddgraddu[_qp] *= _shape_tensor[_qp].inverse();
    _ddgraddv[_qp] *= _shape_tensor[_qp].inverse();
    _ddgraddw[_qp] *= _shape_tensor[_qp].inverse();
  }
}

void
MaterialBaseNOSPD::computeProperties()
{
  MechanicsMaterialBasePD::computeProperties();

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpStrain();
}

void
MaterialBaseNOSPD::computeBondStretch()
{
  _total_stretch[0] = _current_length / _origin_length - 1.0;
  _total_stretch[1] = _total_stretch[0];

  for (auto es : _eigenstrains)
    _mechanical_stretch[0] = _total_stretch[0] - 0.5 * ((*es)[0](0, 0) + (*es)[1](0, 0));

  _mechanical_stretch[1] = _mechanical_stretch[0];
}
