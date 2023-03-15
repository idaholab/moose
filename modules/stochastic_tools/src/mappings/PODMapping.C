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
  params.addParam<ReporterName>(
      "solution_storage", "The name of the storage reporter where the snapshots are located.");
  return params;
}

PODMapping::PODMapping(const InputParameters & parameters) : MappingBase(parameters) {}

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
