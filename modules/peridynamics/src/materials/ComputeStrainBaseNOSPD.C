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
  params.addClassDescription("Base material strain class for the peridynamic correspondence model");

  MooseEnum stabilization_option("FORCE BOND_HORIZON");
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
  _deformation_gradient[_qp].zero();
  _deformation_gradient[_qp].addIa(1.0);

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
                       _pdmesh.getNodeAvgSpacing(_current_elem->node_id(_qp)) * _horiz_rad[_qp] /
                       _origin_vec.norm();
      _multi[_qp] = _horiz_rad[_qp] / _origin_vec.norm() * _node_vol[0] * _node_vol[1];
    }
    else if (_stabilization == "BOND_HORIZON")
      computeBondHorizonQpDeformationGradient();
    else
      paramError("stabilization",
                 "Unknown stabilization scheme for peridynamic correspondence model");
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
  const Node * cur_nd = _current_elem->node_ptr(_qp);
  std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(cur_nd->id());
  std::vector<dof_id_type> bonds = _pdmesh.getBonds(cur_nd->id());

  // calculate the shape tensor and prepare the deformation gradient tensor
  Real vol_nb, weight;
  RealGradient origin_vec_nb, current_vec_nb;

  for (unsigned int nb = 0; nb < neighbors.size(); ++nb)
    if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)
    {
      Node * neighbor_nb = _pdmesh.nodePtr(neighbors[nb]);
      vol_nb = _pdmesh.getPDNodeVolume(neighbors[nb]);
      origin_vec_nb = *neighbor_nb - *_pdmesh.nodePtr(cur_nd->id());

      for (unsigned int k = 0; k < _dim; ++k)
        current_vec_nb(k) = origin_vec_nb(k) + _disp_var[k]->getNodalValue(*neighbor_nb) -
                            _disp_var[k]->getNodalValue(*cur_nd);

      weight = _horiz_rad[_qp] / origin_vec_nb.norm();
      for (unsigned int k = 0; k < _dim; ++k)
      {
        for (unsigned int l = 0; l < _dim; ++l)
        {
          _shape2[_qp](k, l) += weight * origin_vec_nb(k) * origin_vec_nb(l) * vol_nb;
          _deformation_gradient[_qp](k, l) +=
              weight * current_vec_nb(k) * origin_vec_nb(l) * vol_nb;
        }
        // calculate derivatives of deformation_gradient w.r.t displacements of node i
        _ddgraddu[_qp](0, k) += -weight * origin_vec_nb(k) * vol_nb;
        _ddgraddv[_qp](1, k) += -weight * origin_vec_nb(k) * vol_nb;
        if (_dim == 3)
          _ddgraddw[_qp](2, k) += -weight * origin_vec_nb(k) * vol_nb;
      }
    }

  // finalize the deformation gradient tensor
  if (MooseUtils::absoluteFuzzyEqual(_shape2[_qp].det(), 0.0))
    mooseError("Singular shape tensor is detected in ComputeStrainBaseNOSPD! Use "
               "SingularShapeTensorEliminatorUserObjectPD to avoid singular shape tensor");

  _deformation_gradient[_qp] *= _shape2[_qp].inverse();
  _ddgraddu[_qp] *= _shape2[_qp].inverse();
  _ddgraddv[_qp] *= _shape2[_qp].inverse();
  _ddgraddw[_qp] *= _shape2[_qp].inverse();
}

void
ComputeStrainBaseNOSPD::computeBondHorizonQpDeformationGradient()
{
  const Node * cur_nd = _current_elem->node_ptr(_qp);
  const Node * end_nd = _current_elem->node_ptr(1 - _qp); // two nodes for edge2 element
  std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(cur_nd->id());
  std::vector<dof_id_type> bonds = _pdmesh.getBonds(cur_nd->id());

  unsigned int nb_index =
      std::find(neighbors.begin(), neighbors.end(), end_nd->id()) - neighbors.begin();
  std::vector<dof_id_type> dg_neighbors = _pdmesh.getDefGradNeighbors(cur_nd->id(), nb_index);

  // calculate the shape tensor and prepare the deformation gradient tensor
  Real vol_nb, weight, dgnodes_vsum = 0.0;
  RealGradient origin_vec_nb, current_vec_nb;

  for (unsigned int nb = 0; nb < dg_neighbors.size(); ++nb)
    if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[nb]])) > 0.5)
    {
      Node * dgneighbor_nb = _pdmesh.nodePtr(neighbors[dg_neighbors[nb]]);
      vol_nb = _pdmesh.getPDNodeVolume(neighbors[dg_neighbors[nb]]);
      dgnodes_vsum += vol_nb;
      origin_vec_nb = *dgneighbor_nb - *_pdmesh.nodePtr(cur_nd->id());

      for (unsigned int k = 0; k < _dim; ++k)
        current_vec_nb(k) = origin_vec_nb(k) + _disp_var[k]->getNodalValue(*dgneighbor_nb) -
                            _disp_var[k]->getNodalValue(*cur_nd);

      weight = _horiz_rad[_qp] / origin_vec_nb.norm();
      for (unsigned int k = 0; k < _dim; ++k)
      {
        for (unsigned int l = 0; l < _dim; ++l)
        {
          _shape2[_qp](k, l) += weight * origin_vec_nb(k) * origin_vec_nb(l) * vol_nb;
          _deformation_gradient[_qp](k, l) +=
              weight * current_vec_nb(k) * origin_vec_nb(l) * vol_nb;
        }
        // calculate derivatives of deformation_gradient w.r.t displacements of node i
        _ddgraddu[_qp](0, k) += -weight * origin_vec_nb(k) * vol_nb;
        _ddgraddv[_qp](1, k) += -weight * origin_vec_nb(k) * vol_nb;
        if (_dim == 3)
          _ddgraddw[_qp](2, k) += -weight * origin_vec_nb(k) * vol_nb;
      }
    }
  // finalize the deformation gradient and its derivatives
  if (MooseUtils::absoluteFuzzyEqual(_shape2[_qp].det(), 0.0))
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
  _multi[_qp] = _horiz_rad[_qp] / _origin_vec.norm() * _node_vol[0] * _node_vol[1] * dgnodes_vsum /
                _horiz_vol[_qp];
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
