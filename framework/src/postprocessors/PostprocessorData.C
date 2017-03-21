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

#include "PostprocessorData.h"
#include "FEProblem.h"

PostprocessorData::PostprocessorData(FEProblemBase & fe_problem)
  : Restartable("values", "PostprocessorData", fe_problem, 0)
{
}

bool
PostprocessorData::hasPostprocessor(const std::string & name)
{
  return (_values.find(name) != _values.end());
}

PostprocessorValue &
PostprocessorData::getPostprocessorValue(const PostprocessorName & name)
{
  PostprocessorValue *& pp_val = _values[name];

  if (pp_val == NULL)
    pp_val = &declareRestartableDataWithObjectName<PostprocessorValue>(name, "values");

  return *pp_val;
}

PostprocessorValue &
PostprocessorData::getPostprocessorValueOld(const PostprocessorName & name)
{
  PostprocessorValue *& pp_val = _values_old[name];

  if (pp_val == NULL)
    pp_val = &declareRestartableDataWithObjectName<PostprocessorValue>(name, "values_old");

  return *pp_val;
}

PostprocessorValue &
PostprocessorData::getPostprocessorValueOlder(const PostprocessorName & name)
{
  PostprocessorValue *& pp_val = _values_older[name];

  if (pp_val == NULL)
    pp_val = &declareRestartableDataWithObjectName<PostprocessorValue>(name, "values_older");

  return *pp_val;
}

void
PostprocessorData::init(const std::string & name)
{
  getPostprocessorValue(name) = 0.0;
  getPostprocessorValueOld(name) = 0.0;
  getPostprocessorValueOlder(name) = 0.0;
}

void
PostprocessorData::storeValue(const std::string & name, PostprocessorValue value)
{
  getPostprocessorValue(name) = value;
}

void
PostprocessorData::copyValuesBack()
{
  for (const auto & it : _values)
  {
    getPostprocessorValueOlder(it.first) = getPostprocessorValueOld(it.first);
    getPostprocessorValueOld(it.first) = getPostprocessorValue(it.first);
  }
}
