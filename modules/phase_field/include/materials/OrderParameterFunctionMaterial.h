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

// Forward Declarations

/**
 * Material base class for materials that provide the switching function
 * \f$ h(\eta) \f$ or the double well function  \f$ g(\eta) \f$.
 * Implement computeQpProperties in the derived classes.
 */
class OrderParameterFunctionMaterial : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  OrderParameterFunctionMaterial(const InputParameters & parameters);

protected:
  /// Coupled variable value for the order parameter \f$ \eta \f$.
  const VariableValue & _eta;
  unsigned int _eta_var;
  VariableName _eta_name;

  /// name of the function of eta (used to generate the material property names)
  std::string _function_name;

  /// Material property to store \f$ f(\eta) \f$
  MaterialProperty<Real> & _prop_f;

  /// Material property to store the derivative \f$ df(\eta)/d\eta \f$
  MaterialProperty<Real> & _prop_df;

  /// Material property to store the second derivative \f$ d^2f(\eta)/d\eta^2 \f$
  MaterialProperty<Real> & _prop_d2f;
};
