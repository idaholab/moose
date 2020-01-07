//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestShapeElementUserObject.h"

#include "NonlinearSystemBase.h"

registerMooseObject("MooseTestApp", TestShapeElementUserObject);

InputParameters
TestShapeElementUserObject::validParams()
{
  InputParameters params = ShapeElementUserObject::validParams();
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
  // Using this semi-encapsulated member feels like cheating but hey,
  // it's a test object.
  const FEProblemBase & prob = _ti_feproblem;

  const dof_id_type n_local_dfs = prob.getNonlinearSystemBase().system().n_local_dofs();

  // check if executeJacobian was called for each variable on each MPI
  // process that owns any degrees of freedom.
  if (_fe_problem.currentlyComputingJacobian())
  {
    if ((_execute_mask & 1) == 0 && n_local_dfs)
      mooseError("Never called executeJacobian for variable u.");
    if ((_execute_mask & 2) == 0 && n_local_dfs)
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
