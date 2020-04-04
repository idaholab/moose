//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParametricMaterialBasePD.h"
#include "ElasticityTensorTools.h"

#include "libmesh/quadrature.h"

InputParameters
ParametricMaterialBasePD::validParams()
{
  InputParameters params = MechanicsMaterialBasePD::validParams();
  params.addClassDescription("Base class for peridynamic models based on derived micro moduli");

  params.addParam<bool>("plane_stress", false, "Plane stress problem or not");
  params.addCoupledVar("scalar_out_of_plane_strain",
                       "Scalar variable for strain in the out-of-plane direction");
  params.addParam<Real>("stress_free_temperature", "Stress free temperature");
  params.addParam<Real>("thermal_expansion_coeff",
                        "Value of material thermal expansion coefficient");

  return params;
}

ParametricMaterialBasePD::ParametricMaterialBasePD(const InputParameters & parameters)
  : MechanicsMaterialBasePD(parameters),
    _plane_stress(getParam<bool>("plane_stress")),
    _scalar_out_of_plane_strain_coupled(isCoupledScalar("scalar_out_of_plane_strain")),
    _scalar_out_of_plane_strain(_scalar_out_of_plane_strain_coupled
                                    ? coupledScalarValue("scalar_out_of_plane_strain")
                                    : _zero),
    _temp(2),
    _temp_ref(_has_temp ? getParam<Real>("stress_free_temperature") : 0.0),
    _tec(_has_temp ? getParam<Real>("thermal_expansion_coeff") : 0.0),
    _bond_local_force(declareProperty<Real>("bond_local_force")),
    _bond_local_dfdU(declareProperty<Real>("bond_dfdU")),
    _bond_local_dfdT(declareProperty<Real>("bond_dfdT")),
    _bond_local_dfdE(declareProperty<Real>("bond_local_dfdE")),
    _thermal_expansion_coeff(declareProperty<Real>("thermal_expansion_coeff")),
    _Cijkl(getMaterialProperty<RankFourTensor>("elasticity_tensor"))
{
  if (_dim != 2 && _scalar_out_of_plane_strain_coupled)
    mooseError("scalar strain can ONLY be specified for 2D analysis!");

  if (_plane_stress && _scalar_out_of_plane_strain_coupled)
    mooseError("Scalar strain can ONLY be specified for generalized plane strain case!");
}

void
ParametricMaterialBasePD::computeProperties()
{
  setupMeshRelatedData();     // function from base class
  computeBondCurrentLength(); // current length of a bond from base class

  _temp[0] = _has_temp ? _temp_var->getNodalValue(*_current_elem->node_ptr(0)) : 0.0;
  _temp[1] = _has_temp ? _temp_var->getNodalValue(*_current_elem->node_ptr(1)) : 0.0;

  computeMaterialConstants();
  computePeridynamicsParams();

  for (_qp = 0; _qp < _nnodes; ++_qp)
  {
    computeBondStretch();
    computeBondForce();
  }
}

void
ParametricMaterialBasePD::computeMaterialConstants()
{
  /// get material constants from elasticity tensor
  _youngs_modulus = ElasticityTensorTools::getIsotropicYoungsModulus(_Cijkl[0]);
  _poissons_ratio = ElasticityTensorTools::getIsotropicPoissonsRatio(_Cijkl[0]);
  _shear_modulus = 0.5 * _youngs_modulus / (1.0 + _poissons_ratio);
  if (_dim == 3 || _scalar_out_of_plane_strain_coupled) // general 3D and generalized plane strain
    _bulk_modulus = _youngs_modulus / 3.0 / (1.0 - 2.0 * _poissons_ratio);
  else if (_plane_stress) // plane stress case
    _bulk_modulus = _youngs_modulus / 2.0 / (1.0 - _poissons_ratio);
  else // plane strain case
    _bulk_modulus = _youngs_modulus / 2.0 / (1.0 + _poissons_ratio) / (1.0 - 2.0 * _poissons_ratio);

  if (_dim == 2 && !_plane_stress && !_scalar_out_of_plane_strain_coupled) // plane strain case
    _alpha = _tec * (1.0 + _poissons_ratio);
  else
    _alpha = _tec;
}

void
ParametricMaterialBasePD::computeBondStretch()
{
  _thermal_expansion_coeff[_qp] = _alpha;

  _total_stretch[_qp] = _current_len / _origin_vec.norm() - 1.0;
  _mechanical_stretch[_qp] =
      _total_stretch[_qp] - _alpha * (0.5 * (_temp[0] + _temp[1]) - _temp_ref);
}
