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

template <>
InputParameters
validParams<VectorPostprocessor>()
{
  InputParameters params = validParams<UserObject>();
  params += validParams<OutputInterface>();

  params.addParamNamesToGroup("outputs", "Advanced");
  params.registerBase("VectorPostprocessor");
  return params;
}

VectorPostprocessor::VectorPostprocessor(const InputParameters & parameters)
  : OutputInterface(parameters),
    _vpp_name(MooseUtils::shortName(parameters.get<std::string>("_object_name"))),
    _vpp_fe_problem(parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _vpp_tid(parameters.isParamValid("_tid") ? parameters.get<THREAD_ID>("_tid") : 0)
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
  if (_vpp_tid)
    return _thread_local_vectors.emplace(vector_name, VectorPostprocessorValue()).first->second;
  else
    return _vpp_fe_problem->declareVectorPostprocessorVector(_vpp_name, vector_name);
}
