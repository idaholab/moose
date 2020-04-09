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

// Forward Declarations

class TrussMaterial : public Material
{
public:
  static InputParameters validParams();

  TrussMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeProperties();

  virtual void computeQpStrain() = 0;
  virtual void computeQpStress() = 0;

  std::vector<MooseVariable *> _disp_var;

  /// Base name of the material system
  const std::string _base_name;

  /// Number of displacement variables
  unsigned int _ndisp;
  const VariableValue & _youngs_modulus;

  MaterialProperty<Real> & _total_stretch;
  MaterialProperty<Real> & _elastic_stretch;
  MaterialProperty<Real> & _axial_stress;
  MaterialProperty<Real> & _e_over_l;

  Real _origin_length;
  Real _current_length;
};
