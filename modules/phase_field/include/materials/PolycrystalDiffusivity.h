/****************************************************************/
/*                  DO NOT MODIFY THIS HEADER                   */
/*                           Marmot                             */
/*                                                              */
/*            (c) 2017 Battelle Energy Alliance, LLC            */
/*                     ALL RIGHTS RESERVED                      */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*             Under Contract No. DE-AC07-05ID14517             */
/*             With the U. S. Department of Energy              */
/*                                                              */
/*             See COPYRIGHT for full restrictions              */
/****************************************************************/

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
  const VariableValue & _c;
  VariableName _c_name;

  const MaterialPropertyName _diff_name;
  MaterialProperty<Real> & _diff;
  MaterialProperty<Real> & _dDdc;
  const MaterialProperty<Real> & _hb;
  const MaterialProperty<Real> & _hm;
  const MaterialProperty<Real> & _dhbdc;
  const MaterialProperty<Real> & _dhmdc;

  const Real _diff_bulk;
  const Real _diff_void;
  const Real _diff_surf;
  const Real _diff_gb;
  const Real _s_index;
  const Real _gb_index;
  const Real _b_index;
  const Real _v_index;

  const unsigned int _op_num;
  std::vector<const VariableValue *> _vals;
  std::vector<MaterialProperty<Real> *> _dDdv;

  std::vector<const MaterialProperty<Real> *> _dhbdv;
  std::vector<const MaterialProperty<Real> *> _dhmdv;
};
