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
#include "MaterialProperty.h"

/**
 * Simple material to test SolutionInvalidInterface.
 */
class NonsafeMaterial : public Material
{
public:
  static InputParameters validParams();

  NonsafeMaterial(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  const Real & _input_diffusivity;

  const Real & _threshold;

  MaterialProperty<Real> & _diffusivity;

  const bool _test_different_procs;

  const bool _test_invalid_recover;
  const Real _invalid_after_time;

  const bool _flag_solution_warning;
};
