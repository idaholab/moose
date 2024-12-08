//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeIsotropicElasticityTensorShell.h"
#include "RankFourTensor.h"

#include "libmesh/quadrature.h"
#include "libmesh/utility.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/fe_type.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"

registerMooseObject("SolidMechanicsApp", ADComputeIsotropicElasticityTensorShell);

InputParameters
ADComputeIsotropicElasticityTensorShell::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a plane stress isotropic elasticity tensor.");
  params.addRequiredRangeCheckedParam<Real>("poissons_ratio",
                                            "poissons_ratio >= -1.0 & poissons_ratio < 0.5",
                                            "Poisson's ratio for the material.");
  params.addRequiredRangeCheckedParam<Real>(
      "youngs_modulus", "youngs_modulus > 0.0", "Young's modulus of the material.");
  params.addRequiredParam<std::string>("through_thickness_order",
                                       "Quadrature order in out of plane direction");
  return params;
}

ADComputeIsotropicElasticityTensorShell::ADComputeIsotropicElasticityTensorShell(
    const InputParameters & parameters)
  : Material(parameters),
    _poissons_ratio(getParam<Real>("poissons_ratio")),
    _youngs_modulus(getParam<Real>("youngs_modulus"))
{
  _Cijkl.fillSymmetricIsotropicEandNu(_youngs_modulus, _poissons_ratio);

  // correction for plane stress
  _Cijkl(0, 0, 0, 0) = _youngs_modulus / (1.0 - _poissons_ratio * _poissons_ratio);
  _Cijkl(1, 1, 1, 1) = _Cijkl(0, 0, 0, 0);
  _Cijkl(0, 0, 1, 1) = _Cijkl(0, 0, 0, 0) * _poissons_ratio;
  _Cijkl(1, 1, 0, 0) = _Cijkl(0, 0, 1, 1);
  _Cijkl(0, 0, 2, 2) = 0.0;
  _Cijkl(1, 1, 2, 2) = 0.0;
  _Cijkl(2, 2, 2, 2) = 0.0;
  _Cijkl(2, 2, 0, 0) = 0.0;
  _Cijkl(2, 2, 1, 1) = 0.0;

  // get number of quadrature points along thickness based on order
  std::unique_ptr<libMesh::QGauss> t_qrule = std::make_unique<libMesh::QGauss>(
      1, Utility::string_to_enum<Order>(getParam<std::string>("through_thickness_order")));
  _t_points = t_qrule->get_points();
  _elasticity_tensor.resize(_t_points.size());
  _ge.resize(_t_points.size());
  for (unsigned int t = 0; t < _t_points.size(); ++t)
  {
    _elasticity_tensor[t] =
        &declareADProperty<RankFourTensor>("elasticity_tensor_t_points_" + std::to_string(t));
    _ge[t] = &getADMaterialProperty<RankTwoTensor>("ge_t_points_" + std::to_string(t));
  }
}

void
ADComputeIsotropicElasticityTensorShell::computeQpProperties()
{
  for (unsigned int t = 0; t < _t_points.size(); ++t)
  {
    (*_elasticity_tensor[t])[_qp].zero();
    // compute contravariant elasticity tensor
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
        for (unsigned int k = 0; k < 3; ++k)
          for (unsigned int l = 0; l < 3; ++l)
            for (unsigned int m = 0; m < 3; ++m)
              for (unsigned int n = 0; n < 3; ++n)
                for (unsigned int o = 0; o < 3; ++o)
                  for (unsigned int p = 0; p < 3; ++p)
                    (*_elasticity_tensor[t])[_qp](i, j, k, l) +=
                        (*_ge[t])[_qp](i, m) * (*_ge[t])[_qp](j, n) * (*_ge[t])[_qp](k, o) *
                        (*_ge[t])[_qp](l, p) * _Cijkl(m, n, o, p);
  }
}
