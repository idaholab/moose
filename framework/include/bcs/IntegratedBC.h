/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef INTEGRATEDBC_H
#define INTEGRATEDBC_H

#include "BoundaryCondition.h"
#include "Coupleable.h"
#include "MooseVariable.h"
#include "MaterialPropertyInterface.h"

// libMesh
#include "fe.h"
#include "quadrature.h"

class IntegratedBC;

template<>
InputParameters validParams<IntegratedBC>();

/**
 * Base class for deriving any boundary condition of a integrated type
 */
class IntegratedBC :
  public BoundaryCondition,
  public Coupleable,
  public ScalarCoupleable,
  public MaterialPropertyInterface
{
public:
  IntegratedBC(const std::string & name, InputParameters parameters);

  virtual void computeResidual();
  virtual void computeJacobian();
  /**
   * Computes d-ivar-residual / d-jvar...
   */
  void computeJacobianBlock(unsigned int jvar);

protected:
  const Elem * & _current_elem;                                         ///< current element
  unsigned int & _current_side;                                         ///< current side of the current element
  const Elem * & _current_side_elem;                                    ///< current side element

  const std::vector<Point> & _normals;                                  ///< normals at quadrature points

  unsigned int _qp;                                                     ///< quadrature point index
  QBase * & _qrule;                                                     ///< active quadrature rule
  const std::vector< Point > & _q_point;                                ///< active quadrature points
  const std::vector<Real> & _JxW;                                       ///< transformed Jacobian weights
  const std::vector<Real> & _coord;                                     ///< coordinate transformation
  unsigned int _i, _j;                                                  ///< i-th, j-th index for enumerating test and shape functions

  // shape functions
  const std::vector<std::vector<Real> > & _phi;                         ///< shape function values (in QPs)
  const std::vector<std::vector<RealGradient> > & _grad_phi;            ///< gradients of shape functions (in QPs)
  const std::vector<std::vector<RealTensor> > & _second_phi;            ///< second derivatives of shape functions (in QPs)
  // test functions
  const std::vector<std::vector<Real> > & _test;                        ///< test function values (in QPs)
  const std::vector<std::vector<RealGradient> > & _grad_test;           ///< gradients of test functions  (in QPs)
  const std::vector<std::vector<RealTensor> > & _second_test;           ///< second derivatives of test functions (in QPs)
  // unknown
  const VariableValue & _u;                                             ///< the values of the unknown variable this BC is acting on
  const VariableGradient & _grad_u;                                     ///< the gradient of the unknown variable this BC is acting on
  const VariableSecond & _second_u;                                     ///< the second derivative of the unknown variable this BC is acting on

  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian();
  /**
   * This is the virtual that derived classes should override for computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

};

#endif /* INTEGRATEDBC_H */
