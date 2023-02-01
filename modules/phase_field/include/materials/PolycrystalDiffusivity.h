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
#include "DerivativeMaterialInterface.h"

/**
 * Generates a diffusion function to distinguish between the solid, void, grain boundary,
 * and surface diffusion rates.
 */
class PolycrystalDiffusivity : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  PolycrystalDiffusivity(const InputParameters & parameters);
  virtual void computeQpProperties();

protected:
  ///Variable values for concentration and order parameters
  const VariableValue & _c;
  VariableName _c_name;
  const unsigned int _op_num;
  std::vector<const VariableValue *> _vals;

  ///Mateiral property and its derivatives declarations
  const MaterialPropertyName _diff_name;
  MaterialProperty<Real> & _diff;
  MaterialProperty<Real> * _dDdc;
  std::vector<MaterialProperty<Real> *> _dDdv;

  ///Switching function material and its derivatives
  const MaterialProperty<Real> & _hb;
  const MaterialProperty<Real> & _hm;
  const MaterialProperty<Real> & _dhbdc;
  const MaterialProperty<Real> & _dhmdc;
  std::vector<const MaterialProperty<Real> *> _dhbdv;
  std::vector<const MaterialProperty<Real> *> _dhmdv;

  ///Input parameters
  const Real _diff_bulk;
  const Real _diff_void;
  const Real _diff_surf;
  const Real _diff_gb;
  const Real _s_weight;
  const Real _gb_weight;
  const Real _b_weight;
  const Real _v_weight;
};
