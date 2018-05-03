//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParametricMaterialBasePD.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ParametricMaterialBasePD>()
{
  InputParameters params = validParams<MechanicsMaterialBasePD>();
  params.addClassDescription("Base class for peridynamic models based on derived micro moduli");

  params.addParam<bool>("plane_stress", false, "Plane stress problem or not");
  params.addRequiredParam<Real>("youngs_modulus", "Material constant: Young's modulus");
  params.addRequiredParam<Real>("poissons_ratio", "Material constant: Poisson's ratio");
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
    _youngs_modulus(getParam<Real>("youngs_modulus")),
    _poissons_ratio(getParam<Real>("poissons_ratio")),
    _scalar_out_of_plane_strain_coupled(isCoupledScalar("scalar_out_of_plane_strain")),
    _scalar_out_of_plane_strain(_scalar_out_of_plane_strain_coupled
                                    ? coupledScalarValue("scalar_out_of_plane_strain")
                                    : _zero),
    _temp(2),
    _temp_ref(_has_temp ? getParam<Real>("stress_free_temperature") : 0.0),
    _alpha(_has_temp ? getParam<Real>("thermal_expansion_coeff") : 0.0),
    _bond_force_ij(declareProperty<Real>("bond_force_ij")),
    _bond_dfdU_ij(declareProperty<Real>("bond_dfdU_ij")),
    _bond_dfdT_ij(declareProperty<Real>("bond_dfdT_ij")),
    _bond_dfdE_ij(declareProperty<Real>("bond_dfdE_ij")),
    _elasticity_tensor(declareProperty<RankFourTensor>("elasticity_tensor")),
    _thermal_expansion_coeff(declareProperty<Real>("thermal_expansion_coeff"))
{
  if (_dim != 2 && _scalar_out_of_plane_strain_coupled)
    mooseError("scalar strain can ONLY be specified for 2D analysis!");

  if (_plane_stress && _scalar_out_of_plane_strain_coupled)
    mooseError("Scalar strain can ONLY be specified for generalized plane strain case!");

  if (_dim == 2 && !_plane_stress && !_scalar_out_of_plane_strain_coupled) // plane strain case
    _alpha = _alpha * (1.0 + _poissons_ratio);

  _shear_modulus = 0.5 * _youngs_modulus / (1.0 + _poissons_ratio);
  if (_dim == 3 || _plane_stress)
    _bulk_modulus = _youngs_modulus / _dim / (1.0 - (_dim - 1.0) * _poissons_ratio);
  else // plane strain case
    _bulk_modulus = _youngs_modulus / 2.0 / (1.0 + _poissons_ratio) / (1.0 - 2.0 * _poissons_ratio);

  std::vector<Real> iso_const(2);
  iso_const[0] =
      _youngs_modulus * _poissons_ratio / ((1.0 + _poissons_ratio) * (1.0 - 2.0 * _poissons_ratio));
  iso_const[1] = _youngs_modulus / (2.0 * (1.0 + _poissons_ratio));
  _Cijkl.fillFromInputVector(iso_const, RankFourTensor::symmetric_isotropic);
}

void
ParametricMaterialBasePD::computeProperties()
{
  computeNodalTemperature();
  MechanicsMaterialBasePD::computeProperties();

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    computeQpProperties();
    computeBondForce();
  }
}

void
ParametricMaterialBasePD::computeNodalTemperature()
{
  if (_has_temp)
  {
    _temp[0] = _temp_var->getNodalValue(*_current_elem->get_node(0));
    _temp[1] = _temp_var->getNodalValue(*_current_elem->get_node(1));
  }
  else
  {
    _temp[0] = _temp_ref;
    _temp[1] = _temp_ref;
  }
}

void
ParametricMaterialBasePD::computeQpProperties()
{
  _elasticity_tensor[_qp] = _Cijkl;
  _thermal_expansion_coeff[_qp] = _alpha;
}

void
ParametricMaterialBasePD::computeBondStretch()
{
  _total_stretch[0] = _current_length / _origin_length - 1.0;
  _total_stretch[1] = _total_stretch[0];

  _mechanical_stretch[0] = _total_stretch[0] - _alpha * (0.5 * (_temp[0] + _temp[1]) - _temp_ref);
  _mechanical_stretch[1] = _mechanical_stretch[0];
}
