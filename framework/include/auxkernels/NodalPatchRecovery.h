/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NodalPatchRecovery_H
#define NodalPatchRecovery_H

#include "AuxKernel.h"
#include "FEProblem.h"

class NodalPatchRecovery;

template <>
InputParameters validParams<NodalPatchRecovery>();

class NodalPatchRecovery : public AuxKernel
{
public:
  NodalPatchRecovery(const InputParameters & parameters);
  virtual ~NodalPatchRecovery(){};

protected:
  /**
   * generate all combinations of n choose k
   */
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

  /**
   * reserve space for A, B, P, and prepare required material properties
   */
  virtual void precalculateValue() override;

  /**
   * solve the equation Ac = B
   * where c is the coefficients vector from least square fitting
   * nodal value is computed as nodal value = P^Tc
   */
  virtual void compute() override;

  /**
   * polynomial order
   */
  const unsigned _order;

  /**
   * mesh dimension
   */
  const unsigned _dim;

  FEProblemBase & _fe_problem;
  std::vector<std::vector<unsigned>> _multi_index;
  DenseVector<Number> _P;
  DenseMatrix<Number> _A;
  DenseVector<Number> _B;
  DenseVector<Number> _coef;
};

#endif // NodalPatchRecovery_H
