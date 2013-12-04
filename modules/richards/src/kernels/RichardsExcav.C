/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
  if (_func.value(_t, *_current_node) < 1.0)
    return false;
  else
    return true;
}

Real
RichardsExcav::computeQpResidual()
{
  return _u[_qp] - _p_excav;
}

