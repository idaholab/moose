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

#include "Postprocessor.h"
#include "SubProblem.h"
#include "Conversion.h"

// libMesh includes

template<>
InputParameters validParams<Postprocessor>()
{
  MooseEnum execute_options(SetupInterface::getExecuteOptions());
  execute_options = "timestep";  // set the default

  InputParameters params = validParams<MooseObject>();
  params.addParam<MooseEnum>("execute_on", execute_options, "Set to (residual|timestep|timestep_begin) to execute only at that moment");

  params.addParam<std::string>("output", "both", "The values are: none, screen, file, both (no output, output to screen only, output to files only, output both to screen and files)");
  params.addPrivateParam<std::string>("built_by_action", "add_postprocessor");
  return params;
}

Postprocessor::Postprocessor(const std::string & name, InputParameters parameters) :
    _pp_name(name),
    _output(Moose::stringToEnum<Moose::PPSOutputType>(parameters.get<std::string>("output")))
{
}
