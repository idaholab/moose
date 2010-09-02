/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "CoupledForce.h"

template<>
InputParameters validParams<CoupledForce>()
{
  InputParameters params = validParams<Kernel>();

  params.addRequiredCoupledVar("v", "");
  
  return params;
}

CoupledForce::CoupledForce(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
    _v_var(coupled("v")),
    _v(coupledValue("v"))
  {}

Real
CoupledForce::computeQpResidual()
{
  return -_v[_qp]*_test[_i][_qp];
}

Real
CoupledForce::computeQpJacobian()
{
  return 0;
}

Real
CoupledForce::computeQpOffDiagJacobian(unsigned int jvar)
{
  if(jvar == _v_var)
    return -_phi[_j][_qp]*_test[_i][_qp];    
  return 0.0;
}
