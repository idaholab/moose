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


#include "ControlInterface.h"
#include "InputParameterWarehouse.h"
#include "MooseApp.h"

template<>
InputParameters validParams<ControlInterface>()
{
  return emptyInputParameters();
}


ControlInterface::ControlInterface(const InputParameters & parameters) :
    _ci_parameters(parameters),
    _ci_app(*parameters.get<MooseApp *>("_moose_app")),
    _input_parameter_warehouse(_ci_app.getInputParameterWarehouse()),
    _ci_name(parameters.get<std::string>("_object_name"))
{
}
