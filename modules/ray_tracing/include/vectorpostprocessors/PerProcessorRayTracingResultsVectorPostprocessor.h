//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

class RayTracingStudy;

/**
 * Outputs per-processor metrics from a RayTracingStudy
 */
class PerProcessorRayTracingResultsVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  PerProcessorRayTracingResultsVectorPostprocessor(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// The study to pull data from
  const RayTracingStudy & _study;

  /// The results that were chosen
  const MultiMooseEnum & _results;

  /// For convenience and speed
  processor_id_type _pid;

  /// Vector to hold PIDs
  VectorPostprocessorValue & _pid_values;

  /// The VPP values
  std::map<processor_id_type, VectorPostprocessorValue *> _result_values;
};
