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

#include "OptionallyCoupledForce.h"

template<>
InputParameters validParams<OptionallyCoupledForce>()
{
  InputParameters params = validParams<Kernel>();

  params.addCoupledVar("v", 1, "The coupled variable which provides the force");

  return params;
}

OptionallyCoupledForce::OptionallyCoupledForce(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _v_var(coupled("v")),
    _v(coupledValue("v"))
{
}

Real
OptionallyCoupledForce::computeQpResidual()
{
  return -_v[_qp]*_test[_i][_qp];
}

Real
OptionallyCoupledForce::computeQpJacobian()
{
  return 0;
}

Real
OptionallyCoupledForce::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _v_var)
    return -_phi[_j][_qp]*_test[_i][_qp];
  return 0.0;
}
