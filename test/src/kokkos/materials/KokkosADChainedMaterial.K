//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADChainedMaterial.h"

registerKokkosADMaterial("MooseTestApp", KokkosADChainedMaterial);

InputParameters
KokkosADChainedMaterial::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addClassDescription("Test AD material computing q = p^2 + c v + g . x from an AD property "
                             "p of another material.");
  params.addRequiredParam<MaterialPropertyName>("p", "The AD property of another material");
  params.addCoupledVar("v", 0, "The coupled variable");
  params.addParam<Real>("c", 0, "The coefficient of v");
  params.addParam<RealVectorValue>(
      "g", RealVectorValue(0, 0, 0), "The coefficient vector of the position");
  params.addParam<MaterialPropertyName>("property_name", "q", "The name of the property");
  params.addParam<MultiMooseEnum>(
      "listed_inputs",
      MultiMooseEnum("p v"),
      "The inputs listed, in order, in the declaration of the property; when empty, the property "
      "stores the derivatives with respect to every slot of the material");
  return params;
}

KokkosADChainedMaterial::KokkosADChainedMaterial(const InputParameters & parameters)
  : ADMaterial(parameters),
    _p(getKokkosADMaterialProperty<Real>("p")),
    _v(adCoupledValue("v")),
    _c(getParam<Real>("c")),
    _g(getParam<RealVectorValue>("g")),
    _q(declareKokkosADProperty<Real>("property_name", listedInputs()))
{
}

std::vector<Moose::Kokkos::ADPropertyInput>
KokkosADChainedMaterial::listedInputs() const
{
  std::vector<Moose::Kokkos::ADPropertyInput> inputs;

  for (const auto & input : getParam<MultiMooseEnum>("listed_inputs"))
    if (input == "p")
      inputs.emplace_back(_p);
    else
      inputs.emplace_back(_v);

  return inputs;
}
