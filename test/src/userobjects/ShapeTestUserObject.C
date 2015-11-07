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

#include "ShapeTestUserObject.h"

template<>
InputParameters validParams<ShapeTestUserObject>()
{
  InputParameters params = validParams<ShapeElementUserObject>();
  params.addCoupledVar("u", "first coupled variable");
  params.addRequiredParam<unsigned int>("u_dofs", "Number of degrees of freedom per element for u");
  params.addCoupledVar("v", "second coupled variable");
  params.addRequiredParam<unsigned int>("v_dofs", "Number of degrees of freedom per element for v");
  return params;
}

ShapeTestUserObject::ShapeTestUserObject(const InputParameters & parameters) :
    ShapeElementUserObject(parameters),
    _u_value(coupledValue("u")),
    _u_var(coupled("u")),
    _u_dofs(getParam<unsigned int>("u_dofs")),
    _v_value(coupledValue("v")),
    _v_var(coupled("v")),
    _v_dofs(getParam<unsigned int>("v_dofs"))
{
}

void
ShapeTestUserObject::initialize()
{
  _integral = 0.0;

  // Jacobian term storage is up to the user. One option is using an std::vector
  // We resize it to the total number of DOFs in the system and zero it out.
  // WARNING: this can be large number (smart sparse storage could be a future improvement)
  _jacobian_storage.assign(_subproblem.es().n_dofs(), 0.0);

  _execute_mask = 0;
}

void
ShapeTestUserObject::execute()
{
  //
  // integrate u^2*v over the simulation domain
  //
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
    _integral += _JxW[qp] * _coord[qp] * (_u_value[qp] * _u_value[qp]) * _v_value[qp];
}

void
ShapeTestUserObject::executeJacobian(unsigned int jvar)
{
  // derivative of _integral w.r.t. u_j
  if (jvar == _u_var)
  {
    // internal testing to make sure the _phis are initialized, set flag on call
    if (_phi.size() != _u_dofs)
      mooseError("Shape functions for u are initialized incorrectly. Expected " << _u_dofs << " DOFs, found " << _phi.size());
    _execute_mask |= 1;

    // sum jacobian contributions over quadrature points
    Real sum = 0.0;
    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
      sum += _JxW[qp] * _coord[qp] * (2.0 * _u_value[qp] * _phi[_j][qp]) * _v_value[qp];

    // the user has to store the value of sum in a storage object indexed by global DOF _j_global
    _jacobian_storage[_j_global] += sum;
  }

  // derivative of _jacobi_sum w.r.t. v_j
  else if (jvar == _v_var)
  {
    // internal testing to make sure the _phis are initialized, set flag on call
    if (_phi.size() != _v_dofs)
      mooseError("Shape functions for v are initialized incorrectly");
    _execute_mask |= 2;

    // sum jacobian contributions over quadrature points
    Real sum = 0.0;
    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
      sum += _JxW[qp] * _coord[qp] * (_u_value[qp] * _u_value[qp]) * _phi[_j][qp];

    // the user has to store the value of sum in a storage object indexed by global DOF _j_global
    _jacobian_storage[_j_global] += sum;
  }
}

void
ShapeTestUserObject::finalize()
{
  gatherSum(_integral);
  gatherSum(_jacobian_storage);

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
ShapeTestUserObject::threadJoin(const UserObject & y)
{
  const ShapeTestUserObject & shp_uo = dynamic_cast<const ShapeTestUserObject &>(y);

  _integral += shp_uo._integral;

  mooseAssert(_jacobian_storage.size() == shp_uo._jacobian_storage.size(), "Jacobian storage size is inconsistent across threads");
  for (unsigned int i = 0; i < _jacobian_storage.size(); ++i)
    _jacobian_storage[i] += shp_uo._jacobian_storage[i];

  // we expect the jacobians to be computed in every thread, so we use AND
  _execute_mask &= shp_uo._execute_mask;
}
