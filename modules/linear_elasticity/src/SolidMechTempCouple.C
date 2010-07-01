#include "SolidMechTempCouple.h"

template<>
InputParameters validParams<SolidMechTempCouple>()
{
  InputParameters params = validParams<SolidMech>();
  params.addRequiredCoupledVar("temp", "Coupled Temperature");
  return params;
}

SolidMechTempCouple::SolidMechTempCouple(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :SolidMech(name, moose_system, parameters),
    _temp_var(coupled("temp")),
    _thermal_strain(getMaterialProperty<Real>("thermal_strain")),
    _alpha(getMaterialProperty<Real>("alpha"))
{}

void
SolidMechTempCouple::recomputeCouplingConstants()
  {
    recomputeConstants();

    _c4 = _E/(1.-_nu);

    if( 3 == _dim )
      _c4 = _E/(1.-2.*_nu);
  }
