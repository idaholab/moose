//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMixedGranularityTest.h"

registerKokkosMaterial("MooseTestApp", KokkosMixedGranularityTest);

InputParameters
KokkosMixedGranularityTest::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Declares a subdomain-constant property alongside a per-quadrature-point "
                             "property computed from it.");
  params.addRequiredParam<MaterialPropertyName>("coarse_name",
                                                "Name of the subdomain-constant property");
  params.addRequiredParam<MaterialPropertyName>("fine_name",
                                                "Name of the per-quadrature-point property");
  params.addParam<MooseEnum>("coarse_constant_on",
                             MooseEnum("NONE=0 ELEMENT=1 SUBDOMAIN=2", "SUBDOMAIN"),
                             "Granularity to declare the coarse property at, so that a granularity "
                             "the framework rejects can be requested from an input");
  return params;
}

KokkosMixedGranularityTest::KokkosMixedGranularityTest(const InputParameters & parameters)
  : Material(parameters),
    _coarse(declareKokkosProperty<Real>(
        "coarse_name",
        {},
        getParam<MooseEnum>("coarse_constant_on").getEnum<Moose::Kokkos::PropertyConstantOption>())),
    _fine(declareKokkosProperty<Real>("fine_name"))
{
}

registerKokkosMaterial("MooseTestApp", KokkosMissingGranularityHookTest);

InputParameters
KokkosMissingGranularityHookTest::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Declares a subdomain-constant property without defining the hook that "
                             "would fill it.");
  params.addRequiredParam<MaterialPropertyName>("coarse_name", "Name of the unfilled property");
  params.addRequiredParam<MaterialPropertyName>("fine_name",
                                                "Name of the per-quadrature-point property");
  return params;
}

KokkosMissingGranularityHookTest::KokkosMissingGranularityHookTest(
    const InputParameters & parameters)
  : Material(parameters),
    _coarse(declareKokkosProperty<Real>(
        "coarse_name", {}, Moose::Kokkos::PropertyConstantOption::SUBDOMAIN)),
    _fine(declareKokkosProperty<Real>("fine_name"))
{
}
