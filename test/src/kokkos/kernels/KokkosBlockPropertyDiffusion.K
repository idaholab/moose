//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosBlockPropertyDiffusion.h"

registerKokkosResidualObject("MooseTestApp", KokkosBlockPropertyDiffusion);

InputParameters
KokkosBlockPropertyDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "the name of the material property we are going to use");
  return params;
}

KokkosBlockPropertyDiffusion::KokkosBlockPropertyDiffusion(const InputParameters & parameters)
  : Kernel(parameters)
{
  auto [prop, blocks] =
      getKokkosBlockMaterialProperty<Real>(getParam<MaterialPropertyName>("prop_name"));

  _prop = prop;
  _blocks = blocks;
}

void
KokkosBlockPropertyDiffusion::initialSetup()
{
  _has_prop.create(kokkosMesh().getNumSubdomains());
  _has_prop = false;

  for (const auto block : _blocks)
    _has_prop[kokkosMesh().getContiguousSubdomainID(block)] = true;

  _has_prop.copyToDevice();
}
