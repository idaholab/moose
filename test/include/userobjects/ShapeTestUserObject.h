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


#ifndef SHAPETESTUSEROBJECT_H
#define SHAPETESTUSEROBJECT_H

#include "ShapeElementUserObject.h"

//Forward Declarations
class ShapeTestUserObject;

template<>
InputParameters validParams<ShapeTestUserObject>();

class ShapeTestUserObject :
  public ShapeElementUserObject
{
public:
  ShapeTestUserObject(const InputParameters & parameters);

  virtual ~ShapeTestUserObject() {}

  virtual void initialize();
  virtual void execute();
  virtual void executeJacobian(unsigned int jvar);
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

  Real getJacobiSumValue() const { return _jacobi_sum; }

protected:
  Real _jacobi_sum;
  unsigned int _execute_mask;

  VariableValue & _u_value;
  unsigned int _u_var;
  unsigned int _u_dofs;
  VariableValue & _v_value;
  unsigned int _v_var;
  unsigned int _v_dofs;
};

#endif
