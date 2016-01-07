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
#include "Assembly.h"

template<>
InputParameters validParams<ShapeElementUserObject>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addParam<bool>("compute_jacobians", true, "Compute Jacobians for coupled variables");
  params.addParamNamesToGroup("compute_jacobians", "Advanced");
  return params;
}

ShapeElementUserObject::ShapeElementUserObject(const InputParameters & parameters) :
    ElementUserObject(parameters),
    _phi(_assembly.phi()),
    _grad_phi(_assembly.gradPhi()),
    _currently_computing_jacobian(_fe_problem.currentlyComputingJacobian()),
    _compute_jacobians(getParam<bool>("compute_jacobians"))
{
  mooseWarning("Jacobian calculation in UserObjects is an experimental capability with a potentially unstable interface.");
}

unsigned int
ShapeElementUserObject::coupled(const std::string & var_name, unsigned int comp)
{
  MooseVariable * var = getVar(var_name, comp);

  // add to the set of variables for which executeJacobian will be called
  if (_compute_jacobians && var->kind() == Moose::VAR_NONLINEAR)
    _jacobian_moose_variables.insert(var);

  // return the variable number
  return ElementUserObject::coupled(var_name, comp);
}

void
ShapeElementUserObject::executeJacobianWrapper(unsigned int jvar, const std::vector<dof_id_type> & dof_indices)
{
  for (_j = 0; _j < _phi.size(); ++_j)
  {
    _j_global = dof_indices[_j];
    executeJacobian(jvar);
  }
}
