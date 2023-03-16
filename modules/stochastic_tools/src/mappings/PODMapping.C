//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PODMapping.h"

registerMooseObject("StochasticToolsApp", PODMapping);

InputParameters
PODMapping::validParams()
{
  InputParameters params = MappingBase::validParams();
  params.addParam<UserObjectName>(
      "solution_storage", "The name of the storage reporter where the snapshots are located.");
  return params;
}

PODMapping::PODMapping(const InputParameters & parameters) : MappingBase(parameters)
{
  std::string parallel_storage_name = getParam<std::string>("solution_storage");

  std::vector<UserObject *> reporters;
  _fe_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribName>(parallel_storage_name)
      .queryInto(reporters);

  if (reporters.empty())
    paramError(
        "solution_storage", "Unable to find reporter with name '", parallel_storage_name, "'");
  else if (reporters.size() > 1)
    paramError("solution_storage",
               "We found more than one reporter with the name '",
               parallel_storage_name,
               "'");

  _parallel_storage = dynamic_cast<ParallelSolutionStorage *>(reporters[0]);

  if (!_parallel_storage)
    paramError("solution_storage",
               "The parallel storage reporter is not of type '",
               parallel_storage_name,
               "'");
}

void
PODMapping::buildMapping()
{
  _console << "Something smart" << std::endl;
}

void
PODMapping::map(const DenseVector<Real> & full_order_vector,
                std::vector<Real> & reduced_order_vector) const
{
  _console << "Something smart" << std::endl;
}

void
PODMapping::map(const NumericVector<Number> & full_order_vector,
                std::vector<Real> & reduced_order_vector) const
{
  _console << "Something smart" << std::endl;
}

void
PODMapping::inverse_map(const std::vector<Real> & reduced_order_vector,
                        std::vector<Real> & full_order_vector) const
{
  _console << "Something smart" << std::endl;
}
