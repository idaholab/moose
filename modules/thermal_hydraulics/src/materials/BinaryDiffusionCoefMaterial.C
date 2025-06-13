//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BinaryDiffusionCoefMaterial.h"
#include "FluidProperties.h"
#include "VaporMixtureFluidProperties.h"
#include "SinglePhaseFluidProperties.h"
#include "PhysicalConstants.h"
#include "THMNames.h"

registerMooseObject("ThermalHydraulicsApp", BinaryDiffusionCoefMaterial);

InputParameters
BinaryDiffusionCoefMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<UserObjectName>("vapor_mixture_fp",
                                          "The VaporMixtureFluidProperties object");
  params.addRequiredParam<Real>("primary_collision_diameter",
                                "Collision diameter for the first gas component [m]");
  params.addRequiredParam<Real>("secondary_collision_diameter",
                                "Collision diameter for the second gas component [m]");
  params.addClassDescription("Computes the Stefan-Maxwell binary diffusion coefficient.");

  return params;
}

BinaryDiffusionCoefMaterial::BinaryDiffusionCoefMaterial(const InputParameters & parameters)
  : Material(parameters),

    _fp_mix(getUserObject<VaporMixtureFluidProperties>("vapor_mixture_fp")),
    _fp1(_fp_mix.getPrimaryFluidProperties()),
    _fp2(_fp_mix.getSecondaryFluidProperties()),

    _M1(_fp1.molarMass()),
    _M2(_fp2.molarMass()),

    _collision_diam1(getParam<Real>("primary_collision_diameter")),
    _collision_diam2(getParam<Real>("secondary_collision_diameter")),
    _collision_diam(0.5 * (_collision_diam1 + _collision_diam2)),

    _p(getADMaterialProperty<Real>(THM::PRESSURE)),
    _T(getADMaterialProperty<Real>(THM::TEMPERATURE)),

    _diff_coef(declareADProperty<Real>(THM::MASS_DIFFUSION_COEFFICIENT))
{
}

void
BinaryDiffusionCoefMaterial::computeQpProperties()
{
  const ADReal conc = _p[_qp] / (PhysicalConstants::boltzmann_constant * _T[_qp]);
  _diff_coef[_qp] =
      1.0 / (libMesh::pi * std::pow(_collision_diam, 2) * conc) *
      std::sqrt(2.0 * FluidProperties::_R * _T[_qp] / libMesh::pi * (1.0 / _M1 + 1.0 / _M2));
}
