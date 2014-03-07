/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  This post processor returns the mass value of a region.  It is used
//  to check that mass is conserved
//
#include "RichardsMass.h"

template<>
InputParameters validParams<RichardsMass>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  params.addRequiredParam<UserObjectName>("porepressureNames_UO", "The UserObject that holds the list of porepressure names.");
  params.addClassDescription("Returns the mass in a region.");
  return params;
}

RichardsMass::RichardsMass(const std::string & name, InputParameters parameters) :
    ElementIntegralVariablePostprocessor(name, parameters),

    _pp_name_UO(getUserObject<RichardsPorepressureNames>("porepressureNames_UO")),
    _pvar(_pp_name_UO.pressure_var_num(_var.index())),

    _porosity(getMaterialProperty<Real>("porosity")),
    _sat(getMaterialProperty<std::vector<Real> >("sat")),
    _density(getMaterialProperty<std::vector<Real> >("density"))
{
}

Real
RichardsMass::computeQpIntegral()
{
  return _porosity[_qp]*_density[_qp][_pvar]*_sat[_qp][_pvar];
}
