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
#include "EFieldAdvection.h"
#include "Assembly.h"

registerMooseObject("MooseTestApp", EFieldAdvection);

template <>
InputParameters
validParams<EFieldAdvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addCoupledVar("efield", {1, 1, 1}, "The electric field responsible for charge advection");
  MooseEnum charge("positive negative", "positive");
  params.addParam<MooseEnum>(
      "charge", charge, "Whether our primary variable is positive or negatively charged.");
  params.addParam<Real>("mobility", 1., "The mobility of the charge");
  return params;
}

EFieldAdvection::EFieldAdvection(const InputParameters & parameters)
  : Kernel(parameters),
    _efield_id(coupled("efield")),
    _efield(coupledVectorValue("efield")),
    _efield_coupled(isCoupled("efield")),
    _efield_var(_efield_coupled ? getVectorVar("efield", 0) : nullptr),
    _vector_phi(_efield_coupled ? &_assembly.phi(*_efield_var) : nullptr),
    _mobility(getParam<Real>("mobility"))
{
  MooseEnum charge = getParam<MooseEnum>("charge");
  if (charge == "positive")
    _sgn = 1.;
  else
    _sgn = -1.;
}

Real
EFieldAdvection::computeQpResidual()
{
  return -_grad_test[_i][_qp] * _sgn * _mobility * _efield[_qp] * _u[_qp];
}

Real
EFieldAdvection::computeQpJacobian()
{
  return -_grad_test[_i][_qp] * _sgn * _mobility * _efield[_qp] * _phi[_j][_qp];
}

Real
EFieldAdvection::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _efield_id && _efield_coupled)
    return -_grad_test[_i][_qp] * _sgn * _mobility * (*_vector_phi)[_j][_qp] * _u[_qp];
  else
    return 0;
}
