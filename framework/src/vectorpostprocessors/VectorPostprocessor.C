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

// MOOSE includes
#include "VectorPostprocessor.h"
#include "SubProblem.h"
#include "Conversion.h"
#include "UserObject.h"
#include "VectorPostprocessorData.h"
#include "FEProblem.h"

template<>
InputParameters validParams<VectorPostprocessor>()
{
  InputParameters params = validParams<UserObject>();

  params.addParam<std::vector<OutputName> >("outputs", "Vector of output names were you would like to restrict the output of this VectorPostprocessor (empty outputs to all)");

  params.addParamNamesToGroup("outputs", "Advanced");

  params.registerBase("VectorPostprocessor");
  return params;
}

VectorPostprocessor::VectorPostprocessor(const InputParameters & parameters) :
    _vpp_name(MooseUtils::shortName(parameters.get<std::string>("_object_name"))),
    _outputs(parameters.get<std::vector<OutputName> >("outputs")),
    _vpp_fe_problem(parameters.getCheckedPointerParam<FEProblem *>("_fe_problem"))
{
}

VectorPostprocessorValue &
VectorPostprocessor::getVector(const std::string & vector_name)
{
  return _vpp_fe_problem->getVectorPostprocessorValue(_vpp_name, vector_name);
}

VectorPostprocessorValue &
VectorPostprocessor::declareVector(const std::string & vector_name)
{
  return _vpp_fe_problem->declareVectorPostprocessorVector(_vpp_name, vector_name);
}
