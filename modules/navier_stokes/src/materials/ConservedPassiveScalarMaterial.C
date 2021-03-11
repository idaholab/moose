//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservedPassiveScalarMaterial.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", ConservedPassiveScalarMaterial);

InputParameters
ConservedPassiveScalarMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("rho_passive_var",
                               "The passive scalar variable multiplied by the density");
  params.addRequiredParam<MaterialPropertyName>(
      "passive_var_prop",
      "What to name the property computed by dividing the coupled var by the density.");
  return params;
}

ConservedPassiveScalarMaterial::ConservedPassiveScalarMaterial(const InputParameters & parameters)
  : Material(parameters),
    _rho_passive_var(adCoupledValue("rho_passive_var")),
    _passive_var_prop(declareADProperty<Real>("passive_var_prop")),
    _rho(getADMaterialProperty<Real>(NS::density))
{
}

void
ConservedPassiveScalarMaterial::computeQpProperties()
{
  _passive_var_prop[_qp] = _rho_passive_var[_qp] / _rho[_qp];
}
