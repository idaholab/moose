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

#include "TestShapeElementUserObject.h"

template <>
InputParameters
validParams<TestShapeElementUserObject>()
{
  InputParameters params = validParams<ShapeElementUserObject>();
  params.addCoupledVar("u", "first coupled variable");
  params.addRequiredParam<unsigned int>("u_dofs", "Number of degrees of freedom per element for u");
  params.addCoupledVar("v", "second coupled variable");
  params.addRequiredParam<unsigned int>("v_dofs", "Number of degrees of freedom per element for v");
  return params;
}

TestShapeElementUserObject::TestShapeElementUserObject(const InputParameters & parameters)
  : ShapeElementUserObject(parameters),
    _u_var(coupled("u")),
    _u_dofs(getParam<unsigned int>("u_dofs")),
    _v_var(coupled("v")),
    _v_dofs(getParam<unsigned int>("v_dofs"))
{
}

void
TestShapeElementUserObject::initialize()
{
  _execute_mask = 0;
}

void
TestShapeElementUserObject::execute()
{
}

void
TestShapeElementUserObject::executeJacobian(unsigned int jvar)
{
  // derivative of _integral w.r.t. u_j
  if (jvar == _u_var)
  {
    // internal testing to make sure the _phis are initialized, set flag on call
    if (_phi.size() != _u_dofs)
      mooseError("Shape functions for u are initialized incorrectly. Expected ",
                 _u_dofs,
                 " DOFs, found ",
                 _phi.size());
    _execute_mask |= 1;
  }

  // derivative of _jacobi_sum w.r.t. v_j
  else if (jvar == _v_var)
  {
    // internal testing to make sure the _phis are initialized, set flag on call
    if (_phi.size() != _v_dofs)
      mooseError("Shape functions for v are initialized incorrectly");
    _execute_mask |= 2;
  }
}

void
TestShapeElementUserObject::finalize()
{
  // check in all MPI processes if executeJacobian was called for each variable
  if (_fe_problem.currentlyComputingJacobian())
  {
    if ((_execute_mask & 1) == 0)
      mooseError("Never called executeJacobian for variable u.");
    if ((_execute_mask & 2) == 0)
      mooseError("Never called executeJacobian for variable v.");
  }
}

void
TestShapeElementUserObject::threadJoin(const UserObject & y)
{
  const TestShapeElementUserObject & shp_uo = dynamic_cast<const TestShapeElementUserObject &>(y);

  // we expect the jacobians to be computed in every thread, so we use AND
  _execute_mask &= shp_uo._execute_mask;
}
