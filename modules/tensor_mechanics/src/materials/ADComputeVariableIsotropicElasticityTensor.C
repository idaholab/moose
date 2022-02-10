//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeVariableIsotropicElasticityTensor.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", ADComputeVariableIsotropicElasticityTensor);

InputParameters
ADComputeVariableIsotropicElasticityTensor::validParams()
{
  InputParameters params = ADComputeElasticityTensorBase::validParams();
  params.addClassDescription("Compute an isotropic elasticity tensor for elastic constants that "
                             "change as a function of material properties");
  params.addRequiredParam<MaterialPropertyName>(
      "youngs_modulus", "Name of material property defining the Young's Modulus");
  params.addRequiredParam<MaterialPropertyName>(
      "poissons_ratio", "Name of material property defining the Poisson's Ratio");
  return params;
}

ADComputeVariableIsotropicElasticityTensor::ADComputeVariableIsotropicElasticityTensor(
    const InputParameters & parameters)
  : ADComputeElasticityTensorBase(parameters),
    _youngs_modulus(getADMaterialProperty<Real>("youngs_modulus")),
    _poissons_ratio(getADMaterialProperty<Real>("poissons_ratio"))
{
  // all tensors created by this class are always isotropic
  issueGuarantee(_elasticity_tensor_name, Guarantee::ISOTROPIC);
  _ge = &getADMaterialProperty<RankTwoTensor>(_base_name + "ge");
}

void
ADComputeVariableIsotropicElasticityTensor::computeQpElasticityTensor()
{
  // _elasticity_tensor[_qp].fillSymmetricIsotropicEandNu(_youngs_modulus[_qp], _poissons_ratio[_qp]);
  Real E = MetaPhysicL::raw_value(_youngs_modulus[_qp]);
  Real nu = MetaPhysicL::raw_value(_poissons_ratio[_qp]);
  _Cijkl.fillSymmetricIsotropicEandNu(E, nu);
  _Cijkl(0, 0, 0, 0) = E / (1.0 - nu * nu);
  _Cijkl(1, 1, 1, 1) = _Cijkl(0, 0, 0, 0);
  _Cijkl(0, 0, 1, 1) = _Cijkl(0, 0, 0, 0) * nu;
  _Cijkl(1, 1, 0, 0) = _Cijkl(0, 0, 1, 1);
  _Cijkl(0, 0, 2, 2) = 0.0;
  _Cijkl(1, 1, 2, 2) = 0.0;
  _Cijkl(2, 2, 2, 2) = 0.0;
  _Cijkl(2, 2, 0, 0) = 0.0;
  _Cijkl(2, 2, 1, 1) = 0.0;

  _elasticity_tensor[_qp].zero();
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        for (unsigned int l = 0; l < 3; ++l)
          for (unsigned int m = 0; m < 3; ++m)
            for (unsigned int n = 0; n < 3; ++n)
              for (unsigned int o = 0; o < 3; ++o)
                for (unsigned int p = 0; p < 3; ++p)
                  _elasticity_tensor[_qp](i, j, k, l) +=
                      (*_ge)[_qp](i, m) * (*_ge)[_qp](j, n) * (*_ge)[_qp](k, o) *
                      (*_ge)[_qp](l, p) * _Cijkl(m, n, o, p);

}
