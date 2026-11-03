//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADQuadraticMaterial.h"

registerKokkosADMaterial("MooseTestApp", KokkosADQuadraticMaterial);

InputParameters
KokkosADQuadraticMaterial::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addClassDescription("Test AD material computing k = k0 + a T^2 + b S + g . x.");
  params.addRequiredCoupledVar("T", "The coupled variable entering quadratically");
  params.addCoupledVar("S", 0, "The coupled variable entering linearly");
  params.addParam<Real>("k0", 1, "The constant term");
  params.addParam<Real>("a", 1, "The coefficient of the quadratic term in T");
  params.addParam<Real>("b", 0, "The coefficient of S");
  params.addParam<MultiMooseEnum>(
      "listed_inputs",
      MultiMooseEnum("T S"),
      "The inputs listed, in order, in the declaration of the property; when empty, the property "
      "stores the derivatives with respect to every input of the material");
  params.addParam<RealVectorValue>(
      "g", RealVectorValue(0, 0, 0), "The coefficient vector of the position");
  params.addParam<MaterialPropertyName>("property_name", "k", "The name of the property");
  params.addParam<bool>("plain_q_point",
                        false,
                        "Whether to read the position as plain reals, to test that this aborts on "
                        "the current configuration");
  return params;
}

KokkosADQuadraticMaterial::KokkosADQuadraticMaterial(const InputParameters & parameters)
  : ADMaterial(parameters),
    _T(adCoupledValue("T")),
    _S(adCoupledValue("S")),
    _k0(getParam<Real>("k0")),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),
    _g(getParam<RealVectorValue>("g")),
    _plain_q_point(getParam<bool>("plain_q_point")),
    _k(declareKokkosADProperty<Real>("property_name", listedInputs()))
{
}

std::vector<Moose::Kokkos::ADPropertyInput>
KokkosADQuadraticMaterial::listedInputs() const
{
  std::vector<Moose::Kokkos::ADPropertyInput> inputs;

  for (const auto & input : getParam<MultiMooseEnum>("listed_inputs"))
    inputs.emplace_back(input == "T" ? _T : _S);

  return inputs;
}
