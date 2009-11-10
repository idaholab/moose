#include "SolidMechTempCoupleZ.h"

template<>
InputParameters valid_params<SolidMechTempCoupleZ>()
{
  InputParameters params = valid_params<SolidMechTempCouple>();
  return params;
}

SolidMechTempCoupleZ::SolidMechTempCoupleZ(std::string name,
                       InputParameters parameters,
                       std::string var_name,
                       std::vector<std::string> coupled_to,
                       std::vector<std::string> coupled_as)
    :SolidMechTempCouple(name,parameters,var_name,coupled_to,coupled_as)
  {}

Real
SolidMechTempCoupleZ::computeQpResidual()
  {
    recomputeCouplingConstants();

    return -(_c1*(1+2*_c2)*_dphi[_i][_qp](2)*(*_thermal_strain)[_qp]);
  }

Real
SolidMechTempCoupleZ::computeQpOffDiagJacobian(unsigned int jvar)
  {
    recomputeCouplingConstants();
        
    if(jvar == _temp_var)
      return -(_c1*(1+2*_c2)*_dphi[_i][_qp](2)*(*_alpha)[_qp]*_phi[_j][_qp]);
    
    return 0.0;
  }
