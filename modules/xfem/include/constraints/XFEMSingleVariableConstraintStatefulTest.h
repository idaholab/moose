/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMEQUALVALUECONSTRAINTSTATEFULTEST_H
#define XFEMEQUALVALUECONSTRAINTSTATEFULTEST_H

// MOOSE includes
#include "XFEMMaterialManagerConstraint.h"
#include "MooseMesh.h"

// Forward Declarations
class XFEMSingleVariableConstraintStatefulTest;

template <>
InputParameters validParams<XFEMSingleVariableConstraintStatefulTest>();

class XFEMSingleVariableConstraintStatefulTest : public XFEMMaterialManagerConstraint
{
public:
  XFEMSingleVariableConstraintStatefulTest(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual ~XFEMSingleVariableConstraintStatefulTest();

protected:
  /**
   * Set information needed for constraint integration
   */
  virtual void reinitConstraintQuadrature(const ElementPairInfo & element_pair_info) override;

  /**
   *  Compute the residual for one of the constraint quadrature points.
   */
  virtual Real computeQpResidual(Moose::DGResidualType type) override;

  /**
   *  Compute the Jacobian for one of the constraint quadrature points.
   */
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  /// Vector normal to the internal interface
  Point _interface_normal;

  /// Stabilization parameter in Nitsche's formulation
  Real _alpha;

  /// Vector normal to the internal interface
  Real _jump;

  /// Vector normal to the internal interface
  Real _jump_flux;

  const MaterialProperty<Real> * _prop_jump;
  const MaterialProperty<Real> * _prop_jump_old;
};

#endif /* XFEMEQUALVALUECONSTRAINTSTATEFULTEST_H_ */
