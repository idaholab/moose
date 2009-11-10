#include "SolidMechTempCouple.h"

template<>
InputParameters valid_params<SolidMechTempCouple>()
{
  InputParameters params = valid_params<SolidMech>();
  return params;
}

SolidMechTempCouple::SolidMechTempCouple(std::string name,
                      InputParameters parameters,
                      std::string var_name,
                      std::vector<std::string> coupled_to,
                      std::vector<std::string> coupled_as)
    :SolidMech(name,parameters,var_name,coupled_to,coupled_as),
    _temp_var(coupled("temp"))
  {}

void
SolidMechTempCouple::subdomainSetup()
  {
    SolidMech::subdomainSetup(); 
    _thermal_strain = &_material->getRealProperty("thermal_strain");
    _alpha = &_material->getRealProperty("alpha");
  }

void
SolidMechTempCouple::recomputeCouplingConstants()
  {
    recomputeConstants();

    _c4 = _E/(1.-_nu);
    
    if( 3 == _dim )
      _c4 = _E/(1.-2.*_nu);    
  }
