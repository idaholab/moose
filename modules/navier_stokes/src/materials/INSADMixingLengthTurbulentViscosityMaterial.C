//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMixingLengthTurbulentViscosityMaterial.h"

registerMooseObject("NavierStokesApp", INSADMixingLengthTurbulentViscosityMaterial);

InputParameters
INSADMixingLengthTurbulentViscosityMaterial::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addClassDescription(
      "Computes the material corresponding to the turbulent viscosity"
      "for the mixing length model.");
  params.addRequiredCoupledVar("u", "The x-velocity");
  params.addCoupledVar("v", 0, "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", 0, "z-velocity"); // only required in 3D
  params.addRequiredCoupledVar("mixing_length", "Turbulent eddy mixing length.");
  params.addRequiredParam<MaterialPropertyName>("mu", "The viscosity");
  params.addRequiredParam<MaterialPropertyName>("rho", "The value for the density");
  return params;
}

INSADMixingLengthTurbulentViscosityMaterial::INSADMixingLengthTurbulentViscosityMaterial(const InputParameters & parameters)
  : ADMaterial(parameters),
  _mesh_dimension(_mesh.dimension()),
  _grad_u(adCoupledGradient("u")),
  _grad_v(_mesh_dimension >= 2 ? adCoupledGradient("v") : _ad_grad_zero),
  _grad_w(_mesh_dimension == 3 ? adCoupledGradient("w") : _ad_grad_zero),
  _mixing_len(coupledValue("mixing_length")),
  _mu(getADMaterialProperty<Real>("mu")),
  _rho(getADMaterialProperty<Real>("rho")),
  _total_viscosity(declareADProperty<Real>("total_viscosity"))
  {
  }

void
INSADMixingLengthTurbulentViscosityMaterial::computeQpProperties()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  constexpr Real offset = 1e-15; // prevents explosion of sqrt(x) derivative to infinity

  ADReal symmetric_strain_tensor_norm = 2.0 * Utility::pow<2>(_grad_u[_qp](0));
  if (_mesh_dimension >= 2)
  {
      symmetric_strain_tensor_norm += 2.0 * Utility::pow<2>(_grad_v[_qp](1))
                                    + Utility::pow<2>(_grad_v[_qp](0)+_grad_u[_qp](1));
      if (_mesh_dimension >= 3)
        {
          symmetric_strain_tensor_norm += 2.0 * Utility::pow<2>(_grad_w[_qp](2))
                                        + Utility::pow<2>(_grad_u[_qp](2) + _grad_w[_qp](0))
                                        + Utility::pow<2>(_grad_v[_qp](2) + _grad_w[_qp](1));
        }
  }
  symmetric_strain_tensor_norm = std::sqrt(symmetric_strain_tensor_norm + offset);

  // Define turbulent_viscosity
  _total_viscosity[_qp] = _mu[_qp] + _rho[_qp] * symmetric_strain_tensor_norm * _mixing_len[_qp] * _mixing_len[_qp];
#else
  _total_viscosity[_qp] = 0;

#endif
}
