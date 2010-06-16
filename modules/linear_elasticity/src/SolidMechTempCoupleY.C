#include "SolidMechTempCoupleY.h"

template<>
InputParameters validParams<SolidMechTempCoupleY>()
{
  InputParameters params = validParams<SolidMechTempCouple>();
  return params;
}


SolidMechTempCoupleY::SolidMechTempCoupleY(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :SolidMechTempCouple(name, moose_system, parameters)
{}

Real
SolidMechTempCoupleY::computeQpResidual()
{
  recomputeCouplingConstants();

  return -(_c1*(1+2*_c2)*_grad_test[_i][_qp](1)*_thermal_strain[_qp]);
}

Real
SolidMechTempCoupleY::computeQpOffDiagJacobian(unsigned int jvar)
{
  recomputeCouplingConstants();

  if(jvar == _temp_var)
    return -(_c1*(1+2*_c2)*_grad_test[_i][_qp](1)*_alpha[_qp]*_phi[_j][_qp]);
    
  return 0.0;
}
