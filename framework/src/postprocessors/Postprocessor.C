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

  params.addParamNamesToGroup("outputs", "Advanced");
  params.addParam<std::vector<OutputName> >("outputs", "Vector of output names were you would like to restrict the output of this postprocessor (empty outputs to all)");
  params.registerBase("Postprocessor");
  return params;
}

Postprocessor::Postprocessor(const std::string & name, InputParameters parameters) :
    _pp_name(name),
    _outputs(parameters.get<std::vector<OutputName> >("outputs"))
{
}
