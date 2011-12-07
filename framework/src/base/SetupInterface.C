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

#include "SetupInterface.h"
#include "Conversion.h"

template<>
InputParameters validParams<SetupInterface>()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::string>("execute_on", "residual", "Set to (residual|timestep) to execute only at that moment");
  return params;
}

SetupInterface::SetupInterface(InputParameters & params) :
    _exec_flags(Moose::stringToEnum<ExecFlagType>(params.get<std::string>("execute_on")))
{
}

SetupInterface::~SetupInterface()
{
}

