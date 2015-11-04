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
  // all coupled variables of this user object will get iniitialized shape functions
  for (unsigned int i = 0; i < _coupled_moose_vars.size(); ++i)
  {
    MooseVariable * moose_var = _coupled_moose_vars[i];
    if (moose_var->kind() == Moose::VAR_NONLINEAR)
    {
      _assembly.registerUserObjectShapeVariable(moose_var->number());

      if (_requested_jacobian_flag.size() <= moose_var->number())
        _requested_jacobian_flag.resize(moose_var->number() + 1, false);
      _requested_jacobian_flag[moose_var->number()] = true;
    }
  }
}
