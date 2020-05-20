//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSurfaceTensionMaterial.h"
#include "TwoPhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", ADSurfaceTensionMaterial);

InputParameters
ADSurfaceTensionMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<MaterialPropertyName>("T", "Temperature material property");
  params.addRequiredParam<MaterialPropertyName>("surface_tension",
                                                "Name to give surface tension material property");

  params.addRequiredParam<UserObjectName>("fp_2phase",
                                          "Two-phase fluid properties user object name");

  params.addClassDescription("Computes surface tension at some temperature");

  return params;
}

ADSurfaceTensionMaterial::ADSurfaceTensionMaterial(const InputParameters & parameters)
  : Material(parameters),

    _T(getADMaterialProperty<Real>("T")),
    _surface_tension_name(getParam<MaterialPropertyName>("surface_tension")),
    _surface_tension(declareADProperty<Real>(_surface_tension_name)),

    _fp_2phase(getUserObject<TwoPhaseFluidProperties>("fp_2phase"))
{
}

void
ADSurfaceTensionMaterial::computeQpProperties()
{
  _surface_tension[_qp] = _fp_2phase.sigma_from_T(_T[_qp]);
}
