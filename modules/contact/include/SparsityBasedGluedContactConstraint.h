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

#ifndef SPARSITYBASEDGLUEDCONTACTCONSTRAINT_H
#define SPARSITYBASEDGLUEDCONTACTCONSTRAINT_H


//ELK includes
#include "GluedContactConstraint.h"

//Forward Declarations
class SparsityBasedGluedContactConstraint;

template<>
InputParameters validParams<SparsityBasedGluedContactConstraint>();

/**
 * A GluedContactConstraint forces the value of a variable to be the same on both sides of an interface.
 */
class SparsityBasedGluedContactConstraint : public GluedContactConstraint
{
public:
 SparsityBasedGluedContactConstraint(const std::string & name, InputParameters parameters) : GluedContactConstraint(name,parameters){};
  virtual ~SparsityBasedGluedContactConstraint(){}

  /**
   * Compute the constraint Jacobian for the current element.
   * Transfer all of the slave Jacobian columns, not only those based on the mesh connectivity.
   */
  virtual void computeJacobian();

};

#endif
