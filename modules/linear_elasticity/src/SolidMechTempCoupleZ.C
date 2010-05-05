#include "SolidMechTempCoupleZ.h"

template<>
InputParameters validParams<SolidMechTempCoupleZ>()
{
  InputParameters params = validParams<SolidMechTempCouple>();
  return params;
}

SolidMechTempCoupleZ::SolidMechTempCoupleZ(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :SolidMechTempCouple(name, moose_system, parameters)
{}

Real
SolidMechTempCoupleZ::computeQpResidual()
{
  recomputeCouplingConstants();

  return -(_c1*(1+2*_c2)*_dtest[_i][_qp](2)*(*_thermal_strain)[_qp]);
}

Real
SolidMechTempCoupleZ::computeQpOffDiagJacobian(unsigned int jvar)
{
  recomputeCouplingConstants();
        
  if(jvar == _temp_var)
    return -(_c1*(1+2*_c2)*_dtest[_i][_qp](2)*(*_alpha)[_qp]*_phi[_j][_qp]);
    
  return 0.0;
}
