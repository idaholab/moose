#include "SolidMechTempCoupleY.h"

template<>
Parameters valid_params<SolidMechTempCoupleY>()
{
  Parameters params = valid_params<SolidMechTempCouple>();
  return params;
}

SolidMechTempCoupleY::SolidMechTempCoupleY(std::string name,
                       Parameters parameters,
                       std::string var_name,
                       std::vector<std::string> coupled_to,
                       std::vector<std::string> coupled_as)
    :SolidMechTempCouple(name,parameters,var_name,coupled_to,coupled_as)
  {}

Real
SolidMechTempCoupleY::computeQpResidual()
  {
    recomputeCouplingConstants();

    return -(_c1*(1+2*_c2)*_dphi[_i][_qp](1)*(*_thermal_strain)[_qp]);
  }

Real
SolidMechTempCoupleY::computeQpOffDiagJacobian(unsigned int jvar)
  {
    recomputeCouplingConstants();

    if(jvar == _temp_var)
      return -(_c1*(1+2*_c2)*_dphi[_i][_qp](1)*(*_alpha)[_qp]*_phi[_j][_qp]);
    
    return 0.0;
  }
