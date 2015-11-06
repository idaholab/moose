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

#include "ShapeElementUserObject.h"

template<>
InputParameters validParams<ShapeElementUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  return params;
}

ShapeElementUserObject::ShapeElementUserObject(const InputParameters & parameters) :
    ElementUserObject(parameters),
    _phi(_assembly.phi()),
    _grad_phi(_assembly.gradPhi())
{
}

unsigned int
ShapeElementUserObject::coupled(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);

  // Jacobians can only be requested for non-linear variables
  if (var->kind() != Moose::VAR_NONLINEAR)
    mooseError("ShapeElementUserObject Jacobians can only be requested for non-linear variables.");

  // add to the set of variables for which executeJacobian will be called
  _jacobian_moose_variables.insert(var);

  // return the variable number
  return ElementUserObject::coupled(var_name, comp);
}
