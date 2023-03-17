//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "MappingReporter.h"

#include "Sampler.h"

registerMooseObject("StochasticToolsApp", MappingReporter);

InputParameters
MappingReporter::validParams()
{
  InputParameters params = StochasticReporter::validParams();
  params += SamplerInterface::validParams();
  params.addClassDescription("Blabla.");
  params.addRequiredParam<UserObjectName>("mapping", "Name of the mapping object.");
  params.addRequiredParam<std::vector<VariableName>>("variables",
                                                     "The names of the variables whose ");
  params.addRequiredParam<std::string>("parallel_storage", "Something here.");
  params.addParam<SamplerName>("sampler", "Sampler to use for evaluating surrogate models.");
  return params;
}

MappingReporter::MappingReporter(const InputParameters & parameters)
  : StochasticReporter(parameters),
    _sampler(isParamValid("sampler") ? &getSampler("sampler") : nullptr),
    _mapping_name(getParam<UserObjectName>("mapping")),
    _variable_names(getParam<std::vector<VariableName>>("variables"))
{
  std::string parallel_storage_name = getParam<std::string>("parallel_storage");

  std::vector<UserObject *> reporters;
  _fe_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribName>(parallel_storage_name)
      .queryInto(reporters);

  if (reporters.empty())
    paramError(
        "parallel_storage", "Unable to find reporter with name '", parallel_storage_name, "'");
  else if (reporters.size() > 1)
    paramError("parallel_storage",
               "We found more than one reporter with the name '",
               parallel_storage_name,
               "'");

  _parallel_storage = dynamic_cast<ParallelSolutionStorage *>(reporters[0]);

  if (!_parallel_storage)
    paramError("parallel_storage",
               "The parallel storage reporter is not of type '",
               parallel_storage_name,
               "'");

  _vector_real_values.resize(_variable_names.size());
  if (_parallel_storage)
  {
    if (_sampler)
    {
      for (auto var_i : index_range(_variable_names))
      {
        _vector_real_values[var_i] = &declareStochasticReporter<std::vector<Real>>(
            _variable_names[var_i] + "_" + _mapping_name, *_sampler);
      }
    }
  }
}

void
MappingReporter::initialSetup()
{
  std::vector<MappingBase *> mappings;
  _fe_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("MappingBase")
      .condition<AttribName>(_mapping_name)
      .queryInto(mappings);

  if (mappings.empty())
    paramError("mapping", "Unable to find mapping with name '", _mapping_name, "'");
  else if (mappings.size() > 1)
    paramError("mapping", "We found more than one mapping with the name '", _mapping_name, "'");

  _mapping = mappings[0];
}

void
MappingReporter::execute()
{
  _mapping->buildMapping();
  if (_parallel_storage)
  {
    if (_sampler)
    {
      for (const auto sample_i : make_range(_sampler->getNumberOfLocalRows()))
      {
        std::vector<Real> data = _sampler->getNextLocalRow();
        for (const auto var_i : index_range(_variable_names))
        {
          const auto & full_vector = _parallel_storage->getStorage(sample_i, var_i);
          if (full_vector.size() != 1)
            mooseError("MappingReporter is only supported for simulations with one solution field "
                       "per run!");
          _mapping->map(*(full_vector[0]), (*_vector_real_values[var_i])[sample_i]);
        }
      }
    }
  }
}
