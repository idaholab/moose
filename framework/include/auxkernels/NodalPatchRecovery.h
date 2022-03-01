//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "FEProblem.h"

/**
 * This class uses patch recovery technique to recover material properties to smooth nodal fields.
 * It is designed to run on nodal (Lagrange) AuxVariables. However, if it is used with
 * elemental (monomial) AuxVariables, this class falls back on the standard techniques in
 * AuxKernel.C to compute either constant or higher-order monomials, depending on the order of the
 * AuxVariable.
 * Zienkiewicz, O. C. and Zhu, J. Z. (1992), The superconvergent patch recovery and a
 * posteriori error estimates. Part 1: The recovery technique. Int. J. Numer. Meth. Engng., 33:
 * 1331-1364. doi:10.1002/nme.1620330702
 */
class NodalPatchRecovery : public AuxKernel
{
public:
  static InputParameters validParams();

  NodalPatchRecovery(const InputParameters & parameters);
  virtual ~NodalPatchRecovery(){};
  /**
   * This function overrides the one implemented in AuxKernel.C to suppress warnings when retrieving
   * material properties
   *
   * @param name name of the material property
   * @return MaterialProperty holding material property values of type T at all qps on the current
   * element
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name);
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> & getGenericMaterialProperty(const std::string & name);

  /**
   * This function overrides the one implemented in AuxKernel.C to suppress warnings when retrieving
   * material properties
   *
   * @param name name of the material property
   * @return MaterialProperty holding material property values of type T from the previous time step
   * at all qps on the current element
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);
  /**
   * This function overrides the one implemented in AuxKernel.C to suppress warnings when retrieving
   * material properties
   *
   * @param name name of the material property
   * @return MaterialProperty holding material property values of type T from two time steps ago at
   * all qps on the current element
   */
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);

protected:
  /**
   * Find out how many different ways you can choose K items from N items set without repetition and
   * without ordering
   *
   * @param N number of items you can choose from
   * @param K number of items to pick
   * @return a data structure holding all combinations of N choose K
   */
  virtual std::vector<std::vector<unsigned int>> nChooseK(unsigned int N, unsigned int K);

  /**
   * generate a complete multi index table for given dimension and order
   * i.e. given dim = 2, order = 2, generated table will have the following content
   * 0 0
   * 1 0
   * 0 1
   * 2 0
   * 1 1
   * 0 2
   * The first number in each entry represents the order of the first variable, i.e. x;
   * The second number in each entry represents the order of the second variable, i.e. y.
   * Multiplication is implied between numbers in each entry, i.e. 1 1 represents x^1 * y^1
   *
   * @param dim dimension of the multi-index, here dim = mesh dimension
   * @param order generate the multi-index up to certain order
   * @return a data structure holding entries representing the complete multi index
   */
  virtual std::vector<std::vector<unsigned int>> multiIndex(unsigned int dim, unsigned int order);

  /**
   * compute the P vector at given point
   * i.e. given dim = 2, order = 2, polynomial P has the following terms:
   * 1
   * x
   * y
   * x^2
   * xy
   * y^2
   *
   * @param q_point point at which to evaluate the polynomial basis
   */
  virtual void computePVector(Point q_point);

  /**
   * Calculate the patch stiffness matrix as \sum_1^n P^TP
   * where n is the number of quadrature points in the element patch
   */
  virtual void accumulateAMatrix();

  /**
   * calculate the patch load vector as \sum_1^n P^Tval
   * where n is the number of quadrature points in the element patch
   *
   * @param val The variable value to be recovered at the current quadrature point
   */
  virtual void accumulateBVector(Real val);

  /// reserve space for A, B, P, and prepare required material properties
  virtual void reinitPatch();

  /**
   * solve the equation Ac = B
   * where c is the coefficients vector from least square fitting
   * nodal value is computed as nodal value = P^Tc
   */
  virtual void compute() override;

private:
  /// polynomial order, default is variable order
  const unsigned int _patch_polynomial_order;

  FEProblemBase & _fe_problem;
  std::vector<std::vector<unsigned int>> _multi_index;
  DenseVector<Number> _P;
  DenseMatrix<Number> _A;
  DenseVector<Number> _B;
  DenseVector<Number> _coef;
};

template <typename T>
const MaterialProperty<T> &
NodalPatchRecovery::getMaterialProperty(const std::string & name)
{
  return MaterialPropertyInterface::getMaterialProperty<T>(name);
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
NodalPatchRecovery::getGenericMaterialProperty(const std::string & name)
{
  return MaterialPropertyInterface::getGenericMaterialProperty<T, is_ad>(name);
}

template <typename T>
const MaterialProperty<T> &
NodalPatchRecovery::getMaterialPropertyOld(const std::string & name)
{
  return MaterialPropertyInterface::getMaterialPropertyOld<T>(name);
}

template <typename T>
const MaterialProperty<T> &
NodalPatchRecovery::getMaterialPropertyOlder(const std::string & name)
{
  return MaterialPropertyInterface::getMaterialPropertyOlder<T>(name);
}
