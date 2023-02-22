//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSProjection.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSProjection);

InputParameters
INSProjection::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addClassDescription("This class computes the 'projection' part of the 'split' method for "
                             "solving incompressible Navier-Stokes.");
  // Coupled variables
  params.addRequiredCoupledVar("a1", "x-acceleration");
  params.addCoupledVar("a2", "y-acceleration"); // only required in 2D and 3D
  params.addCoupledVar("a3", "z-acceleration"); // only required in 3D
  params.addRequiredCoupledVar(NS::pressure, "pressure");

  // Required parameters
  params.addRequiredParam<unsigned>(
      "component",
      "0,1,2 depending on if we are solving the x,y,z component of the momentum equation");

  // Optional parameters
  params.addParam<MaterialPropertyName>("rho_name", "rho", "density name");

  return params;
}

INSProjection::INSProjection(const InputParameters & parameters)
  : Kernel(parameters),

    // Coupled variables
    _a1(coupledValue("a1")),
    _a2(_mesh.dimension() >= 2 ? coupledValue("a2") : _zero),
    _a3(_mesh.dimension() == 3 ? coupledValue("a3") : _zero),

    // Gradients
    _grad_p(coupledGradient(NS::pressure)),

    // Variable numberings
    _a1_var_number(coupled("a1")),
    _a2_var_number(_mesh.dimension() >= 2 ? coupled("a2") : libMesh::invalid_uint),
    _a3_var_number(_mesh.dimension() == 3 ? coupled("a3") : libMesh::invalid_uint),
    _p_var_number(coupled(NS::pressure)),

    // Required parameters
    _component(getParam<unsigned>("component")),

    // Material properties
    _rho(getMaterialProperty<Real>("rho_name"))
{
}

Real
INSProjection::computeQpResidual()
{
  // Vector object for a
  RealVectorValue a(_a1[_qp], _a2[_qp], _a3[_qp]);

  // Vector object for test function (only the component'th entry is non-zero)
  RealVectorValue test;
  test(_component) = _test[_i][_qp];

  // "Symmetric" part, -a.test
  Real symmetric_part = -a(_component) * _test[_i][_qp];

  // The pressure part, (1/_rho[_qp]) * (grad(p).v)
  Real pressure_part = (1. / _rho[_qp]) * (_grad_p[_qp] * test);

  // Return the result
  return symmetric_part + pressure_part;
}

Real
INSProjection::computeQpJacobian()
{
  // There will be a diagonal component from the time derivative term...
  return 0.;
}

Real
INSProjection::computeQpOffDiagJacobian(unsigned jvar)
{
  if (((jvar == _a1_var_number) && (_component == 0)) ||
      ((jvar == _a2_var_number) && (_component == 1)) ||
      ((jvar == _a3_var_number) && (_component == 2)))
  {
    // The symmetric term's Jacobian is only non-zero when the
    // component of 'a' being differentiated is the same as _component.
    return -_phi[_j][_qp] * _test[_i][_qp];
  }

  else if (jvar == _p_var_number)
  {
    return (1. / _rho[_qp]) * (_grad_phi[_j][_qp](_component) * _test[_i][_qp]);
  }

  else
    return 0;
}
