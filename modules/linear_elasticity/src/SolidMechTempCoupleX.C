#include "SolidMechTempCoupleX.h"

template<>
InputParameters validParams<SolidMechTempCoupleX>()
{
  InputParameters params = validParams<SolidMechTempCouple>();
  return params;
}

SolidMechTempCoupleX::SolidMechTempCoupleX(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :SolidMechTempCouple(name, moose_system, parameters)
{}

Real
SolidMechTempCoupleX::computeQpResidual()
  {
    recomputeCouplingConstants();

    return -(_c1*(1+2*_c2)*_dtest[_i][_qp](0)*(*_thermal_strain)[_qp]);
  }

Real
SolidMechTempCoupleX::computeQpOffDiagJacobian(unsigned int jvar)
  {
    recomputeCouplingConstants();

    if(jvar == _temp_var)
      return -(_c1*(1+2*_c2)*_dtest[_i][_qp](0)*(*_alpha)[_qp]*_phi[_j][_qp]);
    
    return 0.0;
  }
