//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * Integrates material properties over a variable range
 */
template <bool is_ad>
class IntegrationOverVariableRangeMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  IntegrationOverVariableRangeMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  /// Names of the material properties to integrate
  std::vector<std::string> _input_names;

  /// Name of the material properties to declare
  std::vector<std::string> _integrated_names;

  /// Number of properties that will be defined
  unsigned int _num_props;

  /// Variable to read values from
  const GenericVariableValue<is_ad> & _var;
  /// Variable DOFs to reset the variable with
  const VariableValue & _var_dofs;
  /// Variable to re-write at each integration point
  MooseWritableVariable & _writeable_var;

  /// Integration step size
  const Real _dv;

  /// Reference value of the output properties at v=v_ref, such that the integral is over 0 range
  const std::vector<Real> _matprop_ref;

  /// Variable value for which matprop = matprop_ref
  const Real _v_ref;

  /// Vector of grids of property values used for integration (outer ordering is properties, inner is each grid)
  std::vector<std::vector<GenericReal<is_ad>>> _matprop_grid;
  /// Whether to pre-compute the matprop grid once, and re-use it every time
  const bool _precompute_matprop_grid;
  /// Minimum variable value for grid
  const Real _v_grid_min;
  /// Maximum variable value for grid
  const Real _v_grid_max;

  /// Vector of all the input properties
  std::vector<const GenericMaterialProperty<Real, is_ad> *> _properties;
  /// Vector of all the integrated properties
  std::vector<GenericMaterialProperty<Real, is_ad> *> _integrated_properties;

  /// To avoid infinite recursion when recomputing properties. If we dont want to have this we can either:
  /// - do this integration in another system! Auxkernels for example
  /// - pass a list of materials to reinit to avoid doing them all
  bool _looping = false;

private:
  /// Helper to create a grid of material properties
  void computeMatPropGrid(Real v_min, Real v_max, Real dv);
};

typedef IntegrationOverVariableRangeMaterialTempl<false> IntegrationOverVariableRangeMaterial;
typedef IntegrationOverVariableRangeMaterialTempl<true> ADIntegrationOverVariableRangeMaterial;
