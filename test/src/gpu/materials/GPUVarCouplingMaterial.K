//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUVarCouplingMaterial.h"

registerMooseObject("MooseTestApp", KokkosVarCouplingMaterial);

InputParameters
KokkosVarCouplingMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("var", "The variable to be coupled in");
  params.addParam<Real>("base", 0.0, "The baseline of the property");
  params.addParam<Real>("coef", 1.0, "The linear coefficient of the coupled var");
  params.addParam<bool>(
      "declare_old", false, "When True the old value for the material property is declared.");
  params.addParam<TagName>("tag", Moose::SOLUTION_TAG, "The solution vector to be coupled in");
  params.addParam<bool>("use_tag",
                        true,
                        "Whether the coupled value should come from a tag. If false, then we use "
                        "an ordinary coupled value.");
  params.addParam<MaterialPropertyName>(
      "coupled_prop_name",
      "diffusion",
      "The name of the material property that this material declares.");
  return params;
}

KokkosVarCouplingMaterial::KokkosVarCouplingMaterial(const InputParameters & parameters)
  : Material(parameters),
    _var(getParam<bool>("use_tag") ? KokkosCoupledVectorTagValue("var", "tag")
                                   : KokkosCoupledValue("var")),
    _base(getParam<Real>("base")),
    _coef(getParam<Real>("coef")),
    _coupled_prop(declareKokkosProperty<Real>("coupled_prop_name"))
{
  if (getParam<bool>("declare_old"))
    _coupled_prop_old = getKokkosMaterialPropertyOld<Real>("coupled_prop_name");
}
