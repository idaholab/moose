//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumShapeSideUserObject.h"
#include "libmesh/quadrature.h"

registerMooseObject("MooseTestApp", NumShapeSideUserObject);

InputParameters
NumShapeSideUserObject::validParams()
{
  InputParameters params = ShapeSideUserObject::validParams();
  params.addRequiredCoupledVar("u", "Charged species density.");
  return params;
}

NumShapeSideUserObject::NumShapeSideUserObject(const InputParameters & parameters)
  : ShapeSideUserObject(parameters),
    _u_value(coupledValue("u")),
    _u_var(coupled("u")),
    _grad_u(coupledGradient("u"))
{
}

void
NumShapeSideUserObject::initialize()
{
  _integral = 0.0;

  // Jacobian term storage is up to the user. One option is using an std::vector
  // We resize it to the total number of DOFs in the system and zero it out.
  // WARNING: this can be large number (smart sparse storage could be a future improvement)
  if (_fe_problem.currentlyComputingJacobian())
    _jacobian_storage.assign(_subproblem.es().n_dofs(), 0.0);
}

void
NumShapeSideUserObject::execute()
{
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    _integral += _JxW[qp] * _coord[qp] * _grad_u[qp] * _normals[qp];
}

void
NumShapeSideUserObject::executeJacobian(unsigned int jvar)
{
  // derivative of _integral w.r.t. u_j
  if (jvar == _u_var)
  {
    // sum jacobian contributions over quadrature points
    Real sum = 0.0;
    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
      sum += _JxW[qp] * _coord[qp] * _grad_phi[_j][qp] * _normals[qp];

    // the user has to store the value of sum in a storage object indexed by global DOF _j_global
    _jacobian_storage[_j_global] += sum;
  }
}

void
NumShapeSideUserObject::finalize()
{
  gatherSum(_integral);

  if (_fe_problem.currentlyComputingJacobian())
    gatherSum(_jacobian_storage);
}

void
NumShapeSideUserObject::threadJoin(const UserObject & y)
{
  const NumShapeSideUserObject & shp_uo = dynamic_cast<const NumShapeSideUserObject &>(y);
  _integral += shp_uo._integral;

  if (_fe_problem.currentlyComputingJacobian())
  {
    mooseAssert(_jacobian_storage.size() == shp_uo._jacobian_storage.size(),
                "Jacobian storage size is inconsistent across threads");
    for (unsigned int i = 0; i < _jacobian_storage.size(); ++i)
      _jacobian_storage[i] += shp_uo._jacobian_storage[i];
  }
}
