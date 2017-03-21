/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ORDERPARAMETERFUNCTIONMATERIAL_H
#define ORDERPARAMETERFUNCTIONMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class OrderParameterFunctionMaterial;

template <>
InputParameters validParams<OrderParameterFunctionMaterial>();

/**
 * Material base class for materials that provide the switching function
 * \f$ h(\eta) \f$ or the double well function  \f$ g(\eta) \f$.
 * Implement computeQpProperties in the derived classes.
 */
class OrderParameterFunctionMaterial : public DerivativeMaterialInterface<Material>
{
public:
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

#endif // ORDERPARAMETERFUNCTIONMATERIAL_H
