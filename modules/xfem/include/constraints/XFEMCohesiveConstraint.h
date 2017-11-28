/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMCOHESIVECONSTRAINT_H
#define XFEMCOHESIVECONSTRAINT_H

// MOOSE includes
#include "XFEMMaterialManagerConstraint.h"
#include "MooseMesh.h"
// #include "ComputeCohesiveTraction.h"

// Forward Declarations
class XFEMCohesiveConstraint;

template <>
InputParameters validParams<XFEMCohesiveConstraint>();

class XFEMCohesiveConstraint : public XFEMMaterialManagerConstraint
{
public:
  XFEMCohesiveConstraint(const InputParameters & parameters);
  virtual ~XFEMCohesiveConstraint();

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

  virtual void initialSetup() override;

  /// Vector normal to the internal interface
  Point _interface_normal;

  /// Initial stiffness for the cohesive zone
  Real _stiffness;
  /// Max traction
  Real _max_traction;
  /// Strain energy release rate
  Real _Gc;

  const VariableValue & _disp_x;
  const VariableValue & _disp_x_neighbor;
  const VariableValue & _disp_y;
  const VariableValue & _disp_y_neighbor;
  // const VariableValue & _disp_z;
  // const VariableValue & _disp_z_neighbor;

  const unsigned int _component;
  // const ComputeCohesiveTraction & _comp_trac;

  const MaterialProperty<Real> * _max_normal_separation_old;

  const std::string _base_name;
};

#endif /* XFEMCOHESIVECONSTRAINT_H */
