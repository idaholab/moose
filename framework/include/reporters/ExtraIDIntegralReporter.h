//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "ExtraIDIntegralVectorPostprocessor.h"

class ExtraIDIntegralReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  ExtraIDIntegralReporter(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override {}

  struct ExtraIDData
  {
    dof_id_type num_names;
    std::vector<ExtraElementIDName> names;
    std::vector<std::vector<dof_id_type>> unique_ids;

    std::vector<VariableName> variables;
    std::vector<VectorPostprocessorValue *> integrals;
  };

private:
  ExtraIDData & _extra_id_data;
  const ExtraIDIntegralVectorPostprocessor * _vpp_object;
};

void to_json(nlohmann::json & json, const ExtraIDIntegralReporter::ExtraIDData & extra_id_data);
void dataStore(std::ostream &, ExtraIDIntegralReporter::ExtraIDData &, void *);
void dataLoad(std::istream &, ExtraIDIntegralReporter::ExtraIDData &, void *);
