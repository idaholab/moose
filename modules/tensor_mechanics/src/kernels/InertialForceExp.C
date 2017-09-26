/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "InertialForceExp.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "MooseVariable.h"
// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<InertialForceExp>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Calculates the residual for the interial force "
                             "(M*accel) and the contribution of mass dependent "
                             "Rayleigh damping and HHT time integration scheme "
                             "[eta*M*((1+alpha)vel-alpha*vel_old)]");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<bool>("use_lumped_mass", false, "indicate whether use lumped mass matrix");
  return params;
}

InertialForceExp::InertialForceExp(const InputParameters & parameters)
  : Kernel(parameters),
    _density(getMaterialProperty<Real>("density")),
    _lumped(getParam<bool>("use_lumped_mass")),
    _u_old(valueOld()),
    _u_older(valueOlder()),
    _u_nodal(_var.nodalValue()),
    _u_nodal_old(_var.nodalValueOld()),
    _u_nodal_older(_var.nodalValueOlder())
{
}

Real
InertialForceExp::computeQpResidual()
{

  Real accel = 0.0;
  if (_lumped)
    accel += 1. / (_dt * _dt) * (_u_nodal[_qp] - _u_nodal_old[_qp] * 2.0 + _u_nodal_older[_qp]);
  else
    accel += 1. / (_dt * _dt) * (_u[_qp] - _u_old[_qp] * 2.0 + _u_older[_qp]);

  return _test[_i][_qp] * _density[_qp] * accel;
}

Real
InertialForceExp::computeQpJacobian()
{
  if (_lumped)
    return _test[_i][_qp] * _density[_qp] / (_dt * _dt);
  else
    return _test[_i][_qp] * _density[_qp] / (_dt * _dt) * _phi[_j][_qp];
}

void
InertialForceExp::computeJacobian()
{
  if (_lumped)
  {
    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
    for (_i = 0; _i < _test.size(); _i++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        ke(_i, _i) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();
  }
  else
  {

    Kernel::computeJacobian();
  }
}
