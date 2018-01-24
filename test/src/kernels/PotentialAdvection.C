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
#include "PotentialAdvection.h"

template <>
InputParameters
validParams<PotentialAdvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addCoupledVar("potential", "The potential responsible for charge advection");
  params.addParam<bool>("positive_charge",
                        true,
                        "Whether the potential is advecting positive "
                        "charges. Should be set to false if charges are "
                        "negative.");
  return params;
}

PotentialAdvection::PotentialAdvection(const InputParameters & parameters)
  : Kernel(parameters),
    _potential_id(coupled("potential")),
    _sgn(getParam<bool>("positive_charge") ? 1 : -1),
    _default(_fe_problem.getMaxQps(), RealGradient(-1.)),
    _grad_potential(isCoupled("potential") ? coupledGradient("potential") : _default)
{
}

PotentialAdvection::~PotentialAdvection() { _default.release(); }

Real
PotentialAdvection::computeQpResidual()
{
  return -_grad_test[_i][_qp] * _sgn * -_grad_potential[_qp] * _u[_qp];
}

Real
PotentialAdvection::computeQpJacobian()
{
  return -_grad_test[_i][_qp] * _sgn * -_grad_potential[_qp] * _phi[_j][_qp];
}

Real
PotentialAdvection::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _potential_id)
    return -_grad_test[_i][_qp] * _sgn * -_grad_phi[_j][_qp] * _u[_qp];
  else
    return 0;
}
