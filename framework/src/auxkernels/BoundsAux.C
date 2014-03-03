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

#include "BoundsAux.h"
#include "SystemBase.h"

template<>
InputParameters validParams<BoundsAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<Real>("upper", "The upper bound for the variable");
  params.addParam<Real>("lower", "The lower bound for the variable");
  params.addRequiredCoupledVar("bounded_variable", "The variable to be bounded");
  return params;
}

BoundsAux::BoundsAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _upper_vector(_nl_sys.getVector("upper_bound")),
    _lower_vector(_nl_sys.getVector("lower_bound")),
    _bounded_variable_id(coupled("bounded_variable"))
{
  if (!isNodal())
    mooseError("BoundsAux must be used on a nodal auxiliary variable!");
  _upper_valid = parameters.isParamValid("upper");
  if (_upper_valid) _upper = getParam<Real>("upper");
  _lower_valid = parameters.isParamValid("lower");
  if (_lower_valid) _lower = getParam<Real>("lower");
}

Real
BoundsAux::computeValue()
{
  // The zero is for the component, this will only work for Lagrange variables!
  dof_id_type dof = _current_node->dof_number(_nl_sys.number(), _bounded_variable_id, 0);
  if (_upper_valid) _upper_vector.set(dof, _upper);
  if (_lower_valid) _lower_vector.set(dof, _lower);

  return 0.0;
}
