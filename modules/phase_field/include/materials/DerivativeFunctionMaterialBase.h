/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DERIVATIVEFUNCTIONMATERIALBASE_H
#define DERIVATIVEFUNCTIONMATERIALBASE_H

#include "FunctionMaterialBase.h"

// Forward Declarations
class DerivativeFunctionMaterialBase;

template <>
InputParameters validParams<DerivativeFunctionMaterialBase>();

/**
 * %Material base class central to compute the a phase free energy and
 * its derivatives. Classes derived from this base class are central to
 * the KKS system. The calculation of free energies is centralized here
 * as the results are used in multiple kernels (KKSPhaseChemicalPotential
 * and \ref KKSCHBulk).
 *
 * A DerivativeFunctionMaterialBase provides numerous material properties which contain
 * the free energy and its derivatives. The material property names are
 * constructed dynamically by the helper functions propertyNameFirst(),
 * propertyNameSecond(), and propertyNameThird() in DerivativeMaterialInterface.
 *
 * A derived class needs to implement the computeF(), computeDF(),
 * computeD2F(), and (optionally) computeD3F() methods.
 *
 * Note that DerivativeParsedMaterial provides a material for which a mathematical
 * free energy expression can be provided in the input file (with the
 * derivatives being calculated automatically).
 *
 * \see KKSPhaseChemicalPotential
 * \see KKSCHBulk
 * \see DerivativeParsedMaterial
 * \see DerivativeMaterialInterface
 */
class DerivativeFunctionMaterialBase : public FunctionMaterialBase
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
   *       the 'third_derivatives' parameter is set to true.
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

#endif // DERIVATIVEFUNCTIONMATERIALBASE_H
