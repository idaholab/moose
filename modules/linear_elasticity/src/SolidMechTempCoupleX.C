#include "SolidMechTempCoupleX.h"

template<>
InputParameters validParams<SolidMechTempCoupleX>()
{
  InputParameters params = validParams<SolidMechTempCouple>();
  return params;
}

SolidMechTempCoupleX::SolidMechTempCoupleX(std::string name,
                       InputParameters parameters,
                       std::string var_name,
                       std::vector<std::string> coupled_to,
                       std::vector<std::string> coupled_as)
    :SolidMechTempCouple(name,parameters,var_name,coupled_to,coupled_as)
  {}

Real
SolidMechTempCoupleX::computeQpResidual()
  {
    recomputeCouplingConstants();

    return -(_c1*(1+2*_c2)*_dphi[_i][_qp](0)*(*_thermal_strain)[_qp]);
  }

Real
SolidMechTempCoupleX::computeQpOffDiagJacobian(unsigned int jvar)
  {
    recomputeCouplingConstants();

    if(jvar == _temp_var)
      return -(_c1*(1+2*_c2)*_dphi[_i][_qp](0)*(*_alpha)[_qp]*_phi[_j][_qp]);
    
    return 0.0;
  }
