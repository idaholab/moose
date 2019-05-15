//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBCBase.h"
#include "MooseVariableInterface.h"
#include "MooseVariableScalar.h"

// Forward declarations
class ArrayIntegratedBC;

template <>
InputParameters validParams<ArrayIntegratedBC>();

/**
 * Base class for deriving any boundary condition of a integrated type
 */
class ArrayIntegratedBC : public IntegratedBCBase, public MooseVariableInterface<RealEigenVector>
{
public:
  ArrayIntegratedBC(const InputParameters & parameters);

  virtual ArrayMooseVariable & variable() override { return _var; }

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  /**
   * Computes d-ivar-residual / d-jvar...
   */
  virtual void computeJacobianBlock(MooseVariableFEBase & jvar) override;
  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar The number of the scalar variable
   */
  void computeJacobianBlockScalar(unsigned int jvar) override;

protected:
  /**
   * Method for computing the residual at quadrature points
   */
  virtual RealEigenVector computeQpResidual() = 0;

  /**
   * Method for computing the diagonal Jacobian at quadrature points
   */
  virtual RealEigenVector computeQpJacobian() { return RealEigenVector::Zero(_var.count()); }

  /**
   * Method for computing an off-diagonal jacobian component at quadrature points.
   */
  virtual RealEigenMatrix computeQpOffDiagJacobian(MooseVariableFEBase & jvar)
  {
    if (jvar.number() == _var.number())
    {
      RealEigenVector v = computeQpJacobian();
      RealEigenMatrix t = RealEigenMatrix::Zero(_var.count(), _var.count());
      t.diagonal() = v;
      return t;
    }
    else
      return RealEigenMatrix::Zero(_var.count(), jvar.count());
  }

  /**
   * This is the virtual that derived classes should override for computing a full Jacobian
   * component
   */
  virtual RealEigenMatrix computeQpOffDiagJacobianScalar(MooseVariableScalar & jvar)
  {
    return RealEigenMatrix::Zero(_var.count(), (unsigned int)jvar.order() + 1);
  }

  /// for array integrated BC
  void saveLocalArrayResidual(DenseVector<Number> & re,
                              unsigned int i,
                              unsigned int ntest,
                              const RealEigenVector & v)
  {
    for (unsigned int j = 0; j < v.size(); ++j, i += ntest)
      re(i) += v(j);
  }
  void saveDiagLocalArrayJacobian(DenseMatrix<Number> & ke,
                                  unsigned int i,
                                  unsigned int ntest,
                                  unsigned int j,
                                  unsigned int nphi,
                                  const RealEigenVector & v)
  {
    for (unsigned int k = 0; k < v.size(); ++k, i += ntest, j += nphi)
      ke(i, j) += v(k);
  }
  void saveFullLocalArrayJacobian(DenseMatrix<Number> & ke,
                                  unsigned int i,
                                  unsigned int ntest,
                                  unsigned int j,
                                  unsigned int nphi,
                                  const RealEigenMatrix & v)
  {
    unsigned int saved_j = j;
    for (unsigned int k = 0; k < v.rows(); ++k, i += ntest)
    {
      j = saved_j;
      for (unsigned int l = 0; l < v.cols(); ++l, j += nphi)
        ke(i, j) += v(k, l);
    }
  }

  ArrayMooseVariable & _var;

  /// normals at quadrature points
  const MooseArray<Point> & _normals;

  /// shape function values (in QPs)
  const ArrayVariablePhiValue & _phi;

  /// test function values (in QPs)
  const ArrayVariableTestValue & _test;

  /// the values of the unknown variable this BC is acting on
  const ArrayVariableValue & _u;

  /// Number of components of the array variable
  const unsigned int _count;
};
