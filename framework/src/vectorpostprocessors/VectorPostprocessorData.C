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

#include "VectorPostprocessorData.h"
#include "FEProblem.h"

VectorPostprocessorData::VectorPostprocessorData(FEProblem & fe_problem) :
    Restartable("values", "VectorPostprocessorData", fe_problem, 0)
{
}

bool
VectorPostprocessorData::hasVectorPostprocessor(const std::string & name)
{
  return (_values.find(name) != _values.end());
}

VectorPostprocessorValue &
VectorPostprocessorData::getVectorPostprocessorValue(const VectorPostprocessorName & vpp_name, const std::string & vector_name)
{
  VectorPostprocessorValue * & pp_val = _values[vpp_name][vector_name];

  if (pp_val == NULL)
    pp_val = &declareRestartableDataWithObjectName<VectorPostprocessorValue>(vpp_name + "_" + vector_name, "values");

  return *pp_val;
}

VectorPostprocessorValue &
VectorPostprocessorData::getVectorPostprocessorValueOld(const VectorPostprocessorName & vpp_name, const std::string & vector_name)
{
  VectorPostprocessorValue * & pp_val = _values_old[vpp_name][vector_name];

  if (pp_val == NULL)
    pp_val = &declareRestartableDataWithObjectName<VectorPostprocessorValue>(vpp_name + "_" + vector_name, "values_old");

  return *pp_val;
}


VectorPostprocessorValue &
VectorPostprocessorData::declareVector(const std::string & vpp_name, const std::string & vector_name)
{
  getVectorPostprocessorValueOld(vpp_name, vector_name);

  return getVectorPostprocessorValue(vpp_name, vector_name);
}

void
VectorPostprocessorData::copyValuesBack()
{
  for (const auto & it : _values)
    for (const auto & vec_it : it.second)
      getVectorPostprocessorValueOld(it.first, vec_it.first) = *(vec_it.second);
}
