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
#include "UserObject.h"

// libMesh includes

template<>
InputParameters validParams<Postprocessor>()
{
  InputParameters params = validParams<UserObject>();

  MooseEnum output_opts("none, auto, screen, file, both", "auto");
  params.addParam<MooseEnum>("output", output_opts,
                             "The values are: none, screen, file, both, auto (no output, output to screen only, output to files only, output both to screen and files, same as both but no warnings if output options conflict.)");

  params.registerBase("Postprocessor");

  params.addParamNamesToGroup("output", "Advanced");

  return params;
}

Postprocessor::Postprocessor(const std::string & name, InputParameters parameters) :
    _pp_name(name),
    _output(Moose::stringToEnum<Moose::PPSOutputType>(parameters.get<MooseEnum>("output")))
{
}
