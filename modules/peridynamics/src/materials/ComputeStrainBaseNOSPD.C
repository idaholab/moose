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

InputParameters
ComputeStrainBaseNOSPD::validParams()
{
  InputParameters params = MechanicsMaterialBasePD::validParams();
  params.addClassDescription(
      "Base material strain class for the peridynamic correspondence models");

  MooseEnum stabilization_option("FORCE BOND_HORIZON_I BOND_HORIZON_II");
  params.addRequiredParam<MooseEnum>(
      "stabilization",
      stabilization_option,
      "Stabilization techniques used for the peridynamic correspondence model");
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
    _stabilization(getParam<MooseEnum>("stabilization")),
    _plane_strain(getParam<bool>("plane_strain")),
    _Cijkl(getMaterialProperty<RankFourTensor>("elasticity_tensor")),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _eigenstrains(_eigenstrain_names.size()),
    _shape2(declareProperty<RankTwoTensor>("rank_two_shape_tensor")),
    _deformation_gradient(declareProperty<RankTwoTensor>("deformation_gradient")),
    _ddgraddu(declareProperty<RankTwoTensor>("ddeformation_gradient_du")),
    _ddgraddv(declareProperty<RankTwoTensor>("ddeformation_gradient_dv")),
    _ddgraddw(declareProperty<RankTwoTensor>("ddeformation_gradient_dw")),
    _total_strain(declareProperty<RankTwoTensor>("total_strain")),
    _mechanical_strain(declareProperty<RankTwoTensor>("mechanical_strain")),
    _multi(declareProperty<Real>("multi")),
    _sf_coeff(declareProperty<Real>("stabilization_force_coeff"))
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
  _deformation_gradient[_qp].setToIdentity();

  if (_qrule->n_points() < 2) // Gauss_Lobatto: order = 2n-3 (n is number of qp)
    mooseError(
        "NOSPD models require Gauss_Lobatto rule and a minimum of 2 quadrature points, i.e., "
        "order of FIRST");
}

void
ComputeStrainBaseNOSPD::computeQpDeformationGradient()
{
  _shape2[_qp].zero();
  _deformation_gradient[_qp].zero();
  _ddgraddu[_qp].zero();
  _ddgraddv[_qp].zero();
  _ddgraddw[_qp].zero();
  _multi[_qp] = 0.0;
  _sf_coeff[_qp] = 0.0;

  if (_dim == 2)
    _shape2[_qp](2, 2) = _deformation_gradient[_qp](2, 2) = 1.0;

  if (_bond_status_var->getElementalValue(_current_elem) > 0.5)
  {
    if (_stabilization == "FORCE")
    {
      computeConventionalQpDeformationGradient();

      _sf_coeff[_qp] = ElasticityTensorTools::getIsotropicYoungsModulus(_Cijkl[_qp]) *
                       _pdmesh.getNodeAverageSpacing(_current_elem->node_id(_qp)) *
                       _horizon_radius[_qp] / _origin_vec.norm();
      _multi[_qp] = _horizon_radius[_qp] / _origin_vec.norm() * _node_vol[0] * _node_vol[1];
    }
    else if (_stabilization == "BOND_HORIZON_I" || _stabilization == "BOND_HORIZON_II")
      computeBondHorizonQpDeformationGradient();
    else
      paramError("stabilization",
                 "Unknown stabilization scheme for peridynamic correspondence model!");
  }
  else
  {
    _shape2[_qp].setToIdentity();
    _deformation_gradient[_qp].setToIdentity();
  }
}

void
ComputeStrainBaseNOSPD::computeConventionalQpDeformationGradient()
{
  std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(_qp));
  std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(_qp));

  // calculate the shape tensor and prepare the deformation gradient tensor
  Real vol_nb, weight_nb;
  RealGradient origin_vec_nb, current_vec_nb;

  for (unsigned int nb = 0; nb < neighbors.size(); ++nb)
    if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)
    {
      vol_nb = _pdmesh.getNodeVolume(neighbors[nb]);
      origin_vec_nb = *_pdmesh.nodePtr(neighbors[nb]) - *_current_elem->node_ptr(_qp);

      for (unsigned int k = 0; k < _dim; ++k)
        current_vec_nb(k) = origin_vec_nb(k) +
                            _disp_var[k]->getNodalValue(*_pdmesh.nodePtr(neighbors[nb])) -
                            _disp_var[k]->getNodalValue(*_current_elem->node_ptr(_qp));

      weight_nb = _horizon_radius[_qp] / origin_vec_nb.norm();
      for (unsigned int k = 0; k < _dim; ++k)
      {
        for (unsigned int l = 0; l < _dim; ++l)
        {
          _shape2[_qp](k, l) += weight_nb * origin_vec_nb(k) * origin_vec_nb(l) * vol_nb;
          _deformation_gradient[_qp](k, l) +=
              weight_nb * current_vec_nb(k) * origin_vec_nb(l) * vol_nb;
        }
        // calculate derivatives of deformation_gradient w.r.t displacements of node nb
        _ddgraddu[_qp](0, k) += -weight_nb * origin_vec_nb(k) * vol_nb;
        _ddgraddv[_qp](1, k) += -weight_nb * origin_vec_nb(k) * vol_nb;
        if (_dim == 3)
          _ddgraddw[_qp](2, k) += -weight_nb * origin_vec_nb(k) * vol_nb;
      }
    }

  // finalize the deformation gradient tensor
  if (_shape2[_qp].det() == 0.)
    mooseError("Singular shape tensor is detected in ComputeStrainBaseNOSPD! Use "
               "SingularShapeTensorEliminatorUserObjectPD to avoid singular shape tensor!");

  _deformation_gradient[_qp] *= _shape2[_qp].inverse();
  _ddgraddu[_qp] *= _shape2[_qp].inverse();
  _ddgraddv[_qp] *= _shape2[_qp].inverse();
  _ddgraddw[_qp] *= _shape2[_qp].inverse();
}

void
ComputeStrainBaseNOSPD::computeBondHorizonQpDeformationGradient()
{
  std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(_qp));
  std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(_qp));

  dof_id_type nb_index =
      std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - _qp)) -
      neighbors.begin();
  std::vector<dof_id_type> dg_neighbors =
      _pdmesh.getBondDeformationGradientNeighbors(_current_elem->node_id(_qp), nb_index);
  Real dg_sub_vol = _pdmesh.getHorizonSubsetVolume(_current_elem->node_id(_qp), nb_index);
  Real dg_sub_vol_sum = _pdmesh.getHorizonSubsetVolumeSum(_current_elem->node_id(_qp));

  // calculate the shape tensor and prepare the deformation gradient tensor
  Real vol_nb, weight_nb;
  RealGradient origin_vec_nb, current_vec_nb;

  for (unsigned int nb = 0; nb < dg_neighbors.size(); ++nb)
    if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[nb]])) > 0.5)
    {
      vol_nb = _pdmesh.getNodeVolume(neighbors[dg_neighbors[nb]]);
      origin_vec_nb = *_pdmesh.nodePtr(neighbors[dg_neighbors[nb]]) -
                      *_pdmesh.nodePtr(_current_elem->node_id(_qp));

      for (unsigned int k = 0; k < _dim; ++k)
        current_vec_nb(k) =
            origin_vec_nb(k) +
            _disp_var[k]->getNodalValue(*_pdmesh.nodePtr(neighbors[dg_neighbors[nb]])) -
            _disp_var[k]->getNodalValue(*_current_elem->node_ptr(_qp));

      weight_nb = _horizon_radius[_qp] / origin_vec_nb.norm();
      for (unsigned int k = 0; k < _dim; ++k)
      {
        for (unsigned int l = 0; l < _dim; ++l)
        {
          _shape2[_qp](k, l) += weight_nb * origin_vec_nb(k) * origin_vec_nb(l) * vol_nb;
          _deformation_gradient[_qp](k, l) +=
              weight_nb * current_vec_nb(k) * origin_vec_nb(l) * vol_nb;
        }
        // calculate derivatives of deformation_gradient w.r.t displacements of node i
        _ddgraddu[_qp](0, k) += -weight_nb * origin_vec_nb(k) * vol_nb;
        _ddgraddv[_qp](1, k) += -weight_nb * origin_vec_nb(k) * vol_nb;
        if (_dim == 3)
          _ddgraddw[_qp](2, k) += -weight_nb * origin_vec_nb(k) * vol_nb;
      }
    }
  // finalize the deformation gradient and its derivatives
  if (_shape2[_qp].det() == 0.)
    computeConventionalQpDeformationGradient(); // this will affect the corresponding jacobian
                                                // calculation
  else
  {
    _deformation_gradient[_qp] *= _shape2[_qp].inverse();
    _ddgraddu[_qp] *= _shape2[_qp].inverse();
    _ddgraddv[_qp] *= _shape2[_qp].inverse();
    _ddgraddw[_qp] *= _shape2[_qp].inverse();
  }

  // force state multiplier
  if (_stabilization == "BOND_HORIZON_I")
    _multi[_qp] = _horizon_radius[_qp] / _origin_vec.norm() * _node_vol[0] * _node_vol[1] *
                  dg_sub_vol / _horizon_vol[_qp];
  else if (_stabilization == "BOND_HORIZON_II")
    _multi[_qp] = _node_vol[_qp] * dg_sub_vol / dg_sub_vol_sum;
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
  _total_stretch[0] = _current_len / _origin_vec.norm() - 1.0;
  _total_stretch[1] = _total_stretch[0];
  _mechanical_stretch[0] = _total_stretch[0];

  Real factor = 1.0;
  if (_plane_strain)
    factor = 1.0 + ElasticityTensorTools::getIsotropicPoissonsRatio(_Cijkl[0]);

  for (auto es : _eigenstrains)
    _mechanical_stretch[0] -= 0.5 * factor * ((*es)[0](2, 2) + (*es)[1](2, 2));

  _mechanical_stretch[1] = _mechanical_stretch[0];
}
