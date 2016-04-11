/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMEQUALVALUECONSTRAINT_H
#define XFEMEQUALVALUECONSTRAINT_H

// MOOSE includes
#include "ElemElemConstraint.h"
#include "MooseMesh.h"

// Forward Declarations
class XFEMSingleVariableConstraint;

template<>
InputParameters validParams<XFEMSingleVariableConstraint>();

class XFEMSingleVariableConstraint : public ElemElemConstraint
{
public:
  XFEMSingleVariableConstraint(const InputParameters & parameters);
  virtual ~XFEMSingleVariableConstraint();

protected:
  /**
   * Set information needed for constraint integration
   */
  virtual void reinitConstraintQuadrature(const ElementPairInfo & element_pair_info);

  /**
   *  Compute the residual for one of the constraint quadrature points.
   */
  virtual Real computeQpResidual(Moose::DGResidualType type);

  /**
   *  Compute the Jacobian for one of the constraint quadrature points.
   */
  virtual Real computeQpJacobian(Moose::DGJacobianType type);

  /// Vector normal to the internal interface
  Point _interface_normal;

  /// Stabilization parameter in Nitsche's formulation
  Real _alpha;

  /// Vector normal to the internal interface
  Real _jump;

  /// Vector normal to the internal interface
  Real _jump_flux;
};

#endif /* XFEMEQUALVALUECONSTRAINT_H_ */
