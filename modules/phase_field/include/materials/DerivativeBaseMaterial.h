#ifndef DERIVATIVEBASEMATERIAL_H
#define DERIVATIVEBASEMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class DerivativeBaseMaterial;

template<>
InputParameters validParams<DerivativeBaseMaterial>();

/**
 * %Material base class central to compute the a phase free energy and
 * its derivatives. Classes derived from this base class are central to
 * the KKS system. The calculation of free energies is centralized here
 * as the results are used in multiple kernels (KKSPhaseChemicalPotential
 * and \ref KKSCHBulk).
 *
 * A DerivativeBaseMaterial provides numerous material properties which contain
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
class DerivativeBaseMaterial : public DerivativeMaterialInterface<Material>
{
public:
  DerivativeBaseMaterial(const std::string & name, InputParameters parameters);

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
  virtual Real computeDF(unsigned int) { return 0.0; }

  /**
   * Override this method to calculate the second derivatives.
   *
   * \f$ \frac{d^2F}{dc_{arg1} dc_{arg2}} \f$
   *
   * @param arg1 The variable the first derivative is taken of
   * @param arg2 The variable the second derivative is taken of
   */
  virtual Real computeD2F(unsigned int, unsigned int) { return 0.0; }

  /**
   * Override this method to calculate the third derivatives.
   *
   * @Note The implementation of this method is optional. It is only evaluated when
   *       the 'third_derivatives' parameter is set to true.
   */
  virtual Real computeD3F(unsigned int, unsigned int, unsigned int) { return 0.0; }

  /**
   * DerivativeBaseMaterial keeps an internal list of all the variables the derivatives are taken w.r.t.
   * We provide the MOOSE variable bames in _arg_names, the libMesh variable numbers in _arg_numbers, and the
   * input file parameter names in _arg_param_names. All are indexed by the argument index.
   * This method returns the argument index for a given the libMesh variable number.
   *
   * This mapping is necessary for internal classes which maintain lists of derivatives indexed by argument index
   * and need to pull from those lists from the computeDF, computeD2F, and computeD3F methods, which receive
   * libMesh variable numbers as parameters.
   */
  unsigned int argIndex(unsigned int i_var) const
  {
    mooseAssert(i_var < _number_of_nl_variables, "Requesting argIndex() for an invalid Moose variable number. Maybe an AuxVariable?");
    mooseAssert(_arg_numbers[_arg_index[i_var]] == i_var, "Requesting argIndex() for a derivative w.r.t. a variable not coupled to.");
    return _arg_index[i_var];
  }

  /// Coupled variables for function arguments
  std::vector<VariableValue *> _args;

  /**
   * Name of the function value material property and used as a base name to
   * concatenate the material property names for the derivatives.
   */
  std::string _F_name;

  /// Flag that indicates if exactly one linear variable is coupled per input file coupling parameter
  bool _mapping_is_unique;

  /// Number of coupled arguments.
  unsigned int _nargs;

  /// String vector of all argument names.
  std::vector<std::string> _arg_names;

  /// Vector of all argument MOOSE variable numbers.
  std::vector<unsigned int> _arg_numbers;

  /// String vector of the input file coupling parameter name for each argument.
  std::vector<std::string> _arg_param_names;

  /// Calculate (and allocate memory for) the third derivatives of the free energy.
  bool _third_derivatives;

  /// Material property to store the function value.
  MaterialProperty<Real> * _prop_F;

  /// Material properties to store the derivatives of f with respect to arg[i]
  std::vector<MaterialProperty<Real> *> _prop_dF;

  /// Material properties to store the second derivatives.
  std::vector<std::vector<MaterialProperty<Real> *> > _prop_d2F;

  /// Material properties to store the third derivatives.
  std::vector<std::vector<std::vector<MaterialProperty<Real> *> > > _prop_d3F;

  /// number of non-linear variable sin the problem
  const unsigned int _number_of_nl_variables;

private:
  /// Vector to look up the internal coupled variable index into _arg_*  through the libMesh variable number
  std::vector<unsigned int> _arg_index;
};

#endif //DERIVATIVEBASEMATERIAL_H
