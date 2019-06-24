//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeStrainBaseNOSPD.h"
#include "ElasticityTensorTools.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ComputeStrainBaseNOSPD>()
{
  InputParameters params = validParams<MechanicsMaterialBasePD>();
  params.addClassDescription(
      "Base class for Self-stabilized Non-Ordinary State-based PeriDynamic (SNOSPD) "
      "correspondence material model");

  params.addParam<bool>("plane_strain",
                        false,
                        "Plane strain problem or not, this will affect the mechanical stretch "
                        "calculation for problem with eigenstrains");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", "List of eigenstrains to be applied in this strain calculation");

  return params;
}

ComputeStrainBaseNOSPD::ComputeStrainBaseNOSPD(const InputParameters & parameters)
  : DerivativeMaterialInterface<MechanicsMaterialBasePD>(parameters),
    _plane_strain(getParam<bool>("plane_strain")),
    _Cijkl(getMaterialProperty<RankFourTensor>("elasticity_tensor")),
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
ComputeStrainBaseNOSPD::initQpStatefulProperties()
{
  _mechanical_strain[_qp].zero();
  _total_strain[_qp].zero();
  _deformation_gradient[_qp].zero();
  _deformation_gradient[_qp].addIa(1.0);
  if (_qrule->n_points() < 2)
    mooseError("NOSPD models require a minimum quadrature order of THIRD");
}

void
ComputeStrainBaseNOSPD::computeQpDeformationGradient()
{
  _shape_tensor[_qp].zero();
  _deformation_gradient[_qp].zero();
  _ddgraddu[_qp].zero();
  _ddgraddv[_qp].zero();
  _ddgraddw[_qp].zero();
  _multi[_qp] = 0.0;

  // for cases when current bond was broken, assign shape tensor and deformation gradient to unity
  if (_bond_status_var.getElementalValue(_current_elem) < 0.5)
  {
    _shape_tensor[_qp] = RankTwoTensor::initIdentity;
    _deformation_gradient[_qp] = _shape_tensor[_qp];
  }
  else
  {
    if (_dim == 2)
      _shape_tensor[_qp](2, 2) = _deformation_gradient[_qp](2, 2) = 1.0;

    const Node * cur_nd = _current_elem->node_ptr(_qp);
    const Node * end_nd = _current_elem->node_ptr(1 - _qp); // two nodes for edge2 element
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(cur_nd->id());
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(cur_nd->id());

    unsigned int nb =
        std::find(neighbors.begin(), neighbors.end(), end_nd->id()) - neighbors.begin();
    std::vector<unsigned int> dg_neighbors = _pdmesh.getDefGradNeighbors(cur_nd->id(), nb);

    // calculate the shape tensor and prepare the deformation gradient tensor
    Real dgnodes_vsum = 0.0;
    RealGradient ori_vec(_dim), cur_vec(_dim);

    for (unsigned int j = 0; j < dg_neighbors.size(); ++j)
      if (_bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[j]])) > 0.5)
      {
        Node * node_j = _pdmesh.nodePtr(neighbors[dg_neighbors[j]]);
        Real vol_j = _pdmesh.getPDNodeVolume(neighbors[dg_neighbors[j]]);
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
            _shape_tensor[_qp](k, l) += _horiz_rad[_qp] / ori_len * ori_vec(k) * ori_vec(l) * vol_j;
            _deformation_gradient[_qp](k, l) +=
                _horiz_rad[_qp] / ori_len * cur_vec(k) * ori_vec(l) * vol_j;
          }
          // calculate derivatives of deformation_gradient w.r.t displacements of node i
          _ddgraddu[_qp](0, k) += -_horiz_rad[_qp] / ori_len * ori_vec(k) * vol_j;
          _ddgraddv[_qp](1, k) += -_horiz_rad[_qp] / ori_len * ori_vec(k) * vol_j;
          if (_dim == 3)
            _ddgraddw[_qp](2, k) += -_horiz_rad[_qp] / ori_len * ori_vec(k) * vol_j;
        }
      }
    // finalize the deformation gradient and its derivatives
    _deformation_gradient[_qp] *= _shape_tensor[_qp].inverse();
    _ddgraddu[_qp] *= _shape_tensor[_qp].inverse();
    _ddgraddv[_qp] *= _shape_tensor[_qp].inverse();
    _ddgraddw[_qp] *= _shape_tensor[_qp].inverse();

    // force state multiplier
    _multi[_qp] = _horiz_rad[_qp] / _origin_length * _node_vol[0] * _node_vol[1] * dgnodes_vsum /
                  _horiz_vol[_qp];
  }
}

void
ComputeStrainBaseNOSPD::computeProperties()
{
  setupMeshRelatedData();     // function from base class
  computeBondCurrentLength(); // current length of a bond from base class
  computeBondStretch();       // stretch of a bond

  for (_qp = 0; _qp < _nnodes; ++_qp)
    computeQpStrain();
}

void
ComputeStrainBaseNOSPD::computeBondStretch()
{
  _total_stretch[0] = _current_length / _origin_length - 1.0;
  _total_stretch[1] = _total_stretch[0];
  _mechanical_stretch[0] = _total_stretch[0];

  Real factor = 1.0;
  if (_plane_strain)
    factor = 1.0 + ElasticityTensorTools::getIsotropicPoissonsRatio(_Cijkl[0]);

  for (auto es : _eigenstrains)
    _mechanical_stretch[0] -= 0.5 * factor * ((*es)[0](2, 2) + (*es)[1](2, 2));

  _mechanical_stretch[1] = _mechanical_stretch[0];
}
