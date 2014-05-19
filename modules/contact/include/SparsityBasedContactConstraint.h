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

#ifndef SPARSITYBASEDCONTACTCONSTRAINT_H
#define SPARSITYBASEDCONTACTCONSTRAINT_H

//MOOSE includes
#include "NodeFaceConstraint.h"

//Forward Declarations
class SparsityBasedContactConstraint;

template<>
InputParameters validParams<SparsityBasedContactConstraint>();

class SparsityBasedContactConstraint : public NodeFaceConstraint
{
public:
 SparsityBasedContactConstraint(const std::string & name, InputParameters parameters) : NodeFaceConstraint(name,parameters){};
  virtual ~SparsityBasedContactConstraint(){}

  virtual Real computeQpSlaveValue()
  {
    mooseError("Unimplemented pure virtual method.  SparsityBasedContactConstraint should only be used as a base class");
    return 0;
  }
  virtual Real computeQpResidual(Moose::ConstraintType /*type*/)
  {
    mooseError("Unimplemented pure virtual method.  SparsityBasedContactConstraint should only be used as a base class");
    return 0;
  }
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType /*type*/)
  {
    mooseError("Unimplemented pure virtual method.  SparsityBasedContactConstraint should only be used as a base class");
    return 0;
  }

  /**
   * Allow the base class' getConnectedDofIndices() function to
   * participate in overload resolution otherwise we get warnings
   * about overloaded virtual functions and "hiding" in debug mode.
   */
  using NodeFaceConstraint::getConnectedDofIndices;

  /**
   * Gets the indices for all dofs conected to the constraint
   * Get indices for all the slave Jacobian columns, not only those based on the mesh connectivity.
   */
  virtual void getConnectedDofIndices();

};

#endif
