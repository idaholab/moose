//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidPropertiesBinaryDiffusionCoefMaterial.h"
#include "VaporMixtureFluidProperties.h"
#include "THMNames.h"

registerMooseObject("ThermalHydraulicsApp", FluidPropertiesBinaryDiffusionCoefMaterial);

InputParameters
FluidPropertiesBinaryDiffusionCoefMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<UserObjectName>("vapor_mixture_fp",
                                          "The VaporMixtureFluidProperties object");
  params.addClassDescription(
      "Computes the binary diffusion coefficient of a gas mixture from its fluid properties.");

  return params;
}

FluidPropertiesBinaryDiffusionCoefMaterial::FluidPropertiesBinaryDiffusionCoefMaterial(
    const InputParameters & parameters)
  : Material(parameters),

    _fp_mix(getUserObject<VaporMixtureFluidProperties>("vapor_mixture_fp")),

    _p(getADMaterialProperty<Real>(THM::PRESSURE)),
    _T(getADMaterialProperty<Real>(THM::TEMPERATURE)),

    _diff_coef(declareADProperty<Real>(THM::MASS_DIFFUSION_COEFFICIENT))
{
  if (_fp_mix.numberOfComponents() != 2)
    paramError("vapor_mixture_fp", "The gas mixture must have exactly two components.");
}

void
FluidPropertiesBinaryDiffusionCoefMaterial::computeQpProperties()
{
  // The primary gas has index 0 and the secondary gas index 1
  _diff_coef[_qp] = _fp_mix.binaryDiffusionCoefficient(_p[_qp], _T[_qp], 0, 1);
}
