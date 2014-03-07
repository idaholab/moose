/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "RichardsExcav.h"
#include "Function.h"

#include <iostream>


template<>
InputParameters validParams<RichardsExcav>()
{
  InputParameters params = validParams<NodalBC>();
  params.addRequiredParam<Real>("p_excav", "Porepressure at the surface of the excavation.  Usually this is atmospheric pressure");
  params.addRequiredParam<FunctionName>("excav_geom_function", "The function describing the excavation geometry (type RichardsExcavGeom)");
  params.addClassDescription("Allows the user to set porepressure at the face of an excavation.  You must have defined the excavation start time, start position, etc, through the excav_geom_function");
  return params;
}

RichardsExcav::RichardsExcav(const std::string & name,
                                             InputParameters parameters) :
    NodalBC(name,parameters),
    _p_excav(getParam<Real>("p_excav")),
    _func(getFunction("excav_geom_function"))
{}

bool
RichardsExcav::shouldApply()
{
  if (_func.value(_t, *_current_node) == 0.0)
    return false;
  else
    return true;
}

Real
RichardsExcav::computeQpResidual()
{
  return _u[_qp] - _p_excav;
}

