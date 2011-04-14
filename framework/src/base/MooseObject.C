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

#include "MooseObject.h"
#include "Conversion.h"

template<>
InputParameters validParams<MooseObject>()
{
  InputParameters params;
  params.addParam<std::string>("execute_on", "residual", "Set to (residual|timestep) to execute only at that moment");
  return params;
}


MooseObject::MooseObject(const std::string & name, InputParameters parameters) :
    _name(name),
    _pars(parameters),
    _exec_flags(Moose::stringToEnum<ExecFlagType>(getParam<std::string>("execute_on")))
{
}
