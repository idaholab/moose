//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALPATCHRECOVERY_H
#define NODALPATCHRECOVERY_H

#include "AuxKernel.h"
#include "FEProblem.h"

/**
 * This class uses patch recovery technique to recover material properties to smooth nodal fields
 * Zienkiewicz, O. C. and Zhu, J. Z. (1992), The superconvergent patch recovery and a posteriori
 * error estimates. Part 1: The recovery technique. Int. J. Numer. Meth. Engng., 33: 1331-1364.
 * doi:10.1002/nme.1620330702
 */
class NodalPatchRecovery;

template <>
InputParameters validParams<NodalPatchRecovery>();

class NodalPatchRecovery : public AuxKernel
{
public:
  NodalPatchRecovery(const InputParameters & parameters);
  virtual ~NodalPatchRecovery(){};
  /// Override from Auxkernel to suppress warnings
  template <typename T>
  const MaterialProperty<T> & getMaterialProperty(const std::string & name);
  /// Override from Auxkernel to suppress warnings
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOld(const std::string & name);
  /// Override from Auxkernel to suppress warnings
  template <typename T>
  const MaterialProperty<T> & getMaterialPropertyOlder(const std::string & name);

protected:
  /// generate all combinations of n choose k
  virtual std::vector<std::vector<unsigned>> nchoosek(unsigned N, unsigned K);

  /**
   * generate a complete multi index table for given dimension and order
   * i.e. given dim = 2, order = 2, generated table will have the following content
   * 0 0
   * 1 0
   * 0 1
   * 2 0
   * 1 1
   * 0 2
   */
  virtual std::vector<std::vector<unsigned>> multiIndex(unsigned dim, unsigned order);

  /**
   * compute the P vector at given point
   * i.e. given dim = 2, order = 2, polynomial P has the following terms:
   * 1
   * x
   * y
   * x^2
   * xy
   * y^2
   */
  virtual void computePVector(Point q_point);

  /**
   * calculate the patch stiffness matrix as \sum_1^n P^TP
   * where n is the number of quadrature points in the element patch
   */
  virtual void accumulateAMatrix();

  /**
   * calculate the patch load vector as \sum_1^n P^Tval
   * where n is the number of quadrature points in the element patch
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

  /// a boolean indicating whether a patch polynomial order is specified by the user
  const bool _order_provided;

  /// polynomial order, default is variable order
  const unsigned _order;

  /// mesh dimension
  const unsigned _dim;

  FEProblemBase & _fe_problem;
  std::vector<std::vector<unsigned>> _multi_index;
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

#endif // NODALPATCHRECOVERY_H
