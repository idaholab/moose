//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionMaterialBase.h"

// Forward Declarations
class DerivativeFunctionMaterialBase;

template <>
InputParameters validParams<DerivativeFunctionMaterialBase>();

/**
 * Material base class to compute a function and its derivatives.
 *
 * A DerivativeFunctionMaterialBase provides numerous material properties which contain
 * the free energy and its derivatives. The material property names are
 * constructed dynamically by the helper functions derivativePropertyNameFirst(),
 * derivativePropertyNameSecond(), and derivativePropertyNameThird() in
 * DerivativeMaterialPropertyNameInterface.
 *
 * A derived class needs to implement the computeF(), computeDF(),
 * computeD2F(), and (optionally) computeD3F() methods.
 *
 * \see DerivativeParsedMaterial
 * \see DerivativeMaterialInterface
 */
class DerivativeFunctionMaterialBase : public FunctionMaterialBase<Real>
{
public:
  DerivativeFunctionMaterialBase(const InputParameters & parameters);

protected:
  virtual void computeProperties();

  /**
   * Check if we got the right number of components in the 'args' coupled
   * variable vector.
   */
  virtual void initialSetup();

  /**
   * Override this method to provide the free energy function.
   */
  virtual Real computeF() { return 0.0; }

  /**
   * Override this method for calculating the first derivatives.
   * The parameter is the libMesh variable number of the coupled variable.
   * These numbers can be obtained using the coupled() method for each coupled variable.
   *
   * @param arg The index of the function argument the derivative is taken of
   */
  virtual Real computeDF(unsigned int arg)
  {
    libmesh_ignore(arg);
    return 0.0;
  }

  /**
   * Override this method to calculate the second derivatives.
   *
   * \f$ \frac{d^2F}{dc_{arg1} dc_{arg2}} \f$
   *
   * @param arg1 The variable the first derivative is taken of
   * @param arg2 The variable the second derivative is taken of
   */
  virtual Real computeD2F(unsigned int arg1, unsigned int arg2)
  {
    libmesh_ignore(arg1);
    libmesh_ignore(arg2);
    return 0.0;
  }

  /**
   * Override this method to calculate the third derivatives.
   *
   * @note The implementation of this method is optional. It is only evaluated when
   *       the 'derivative_order' parameter is set to 3.
   */
  virtual Real computeD3F(unsigned int, unsigned int, unsigned int) { return 0.0; }

  /// Calculate (and allocate memory for) the third derivatives of the free energy.
  bool _third_derivatives;

  /// Material properties to store the derivatives of f with respect to arg[i]
  std::vector<MaterialProperty<Real> *> _prop_dF;

  /// Material properties to store the second derivatives.
  std::vector<std::vector<MaterialProperty<Real> *>> _prop_d2F;

  /// Material properties to store the third derivatives.
  std::vector<std::vector<std::vector<MaterialProperty<Real> *>>> _prop_d3F;
};
