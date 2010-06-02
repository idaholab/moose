#include "SolidMechTempCouple.h"

template<>
InputParameters validParams<SolidMechTempCouple>()
{
  InputParameters params = validParams<SolidMech>();
  return params;
}

SolidMechTempCouple::SolidMechTempCouple(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :SolidMech(name, moose_system, parameters),
    _temp_var(coupled("temp")),
    _thermal_strain(getRealMaterialProperty("thermal_strain")),
    _alpha(getRealMaterialProperty("alpha"))
{}

void
SolidMechTempCouple::recomputeCouplingConstants()
  {
    recomputeConstants();

    _c4 = _E/(1.-_nu);
    
    if( 3 == _dim )
      _c4 = _E/(1.-2.*_nu);    
  }
