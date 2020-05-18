//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

class TwoPhaseFluidProperties;

/**
 * Computes surface tension at some temperature
 */
class ADSurfaceTensionMaterial : public Material
{
public:
  static InputParameters validParams();

  ADSurfaceTensionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Temperature
  const ADMaterialProperty<Real> & _T;
  /// Surface tension material property name
  const MaterialPropertyName & _surface_tension_name;
  /// Surface tension
  ADMaterialProperty<Real> & _surface_tension;

  /// 2-phase fluid properties object
  const TwoPhaseFluidProperties & _fp_2phase;
};
