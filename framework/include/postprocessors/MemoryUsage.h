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

#ifndef MEMORYUSAGE_H
#define MEMORYUSAGE_H

#include "GeneralPostprocessor.h"

class MemoryUsage;

template <>
InputParameters validParams<MemoryUsage>();

/**
 * Output maximum, average, or total process memory usage
 */
class MemoryUsage : public GeneralPostprocessor
{
public:
  MemoryUsage(const InputParameters & parameters);

  virtual void timestepSetup() override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual PostprocessorValue getValue() override;

protected:
  enum class MemType
  {
    virtual_memory,
    physical_memory,
    page_faults
  } _mem_type;

  enum class ValueType
  {
    total,
    average,
    max_process,
    min_process
  } _value_type;

  /// memory usage metric in bytes
  Real _value;

  /// peak memory usage metric in bytes (of multiple samples in the current time step)
  Real _peak_value;

  /// report peak value for multiple samples in a time step
  const bool _report_peak_value;
};

#endif // MEMORYUSAGE_H
