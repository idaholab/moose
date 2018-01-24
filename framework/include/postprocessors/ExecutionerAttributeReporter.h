//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EXECUTIONERATTRIBUTEREPORTER_H
#define EXECUTIONERATTRIBUTEREPORTER_H

// MOOSE includes
#include "GeneralPostprocessor.h"

// Forward declarations
class ExecutionerAttributeReporter;

template <>
InputParameters validParams<ExecutionerAttributeReporter>();

/**
 * A class to report class attributes value from Executioners
 *
 * This postprocessor is only designed to be instantiated by
 * Executioner objects. It will produce an error
 *
 */
class ExecutionerAttributeReporter : public GeneralPostprocessor
{
public:
  /**
   * Class constructor
   * @param parameters
   */
  ExecutionerAttributeReporter(const InputParameters & parameters);

  ///@{
  /**
   * These methods are intentionally empty
   */
  virtual void execute() override {}
  virtual void initialize() override {}
  ///@}

  /**
   * Returns the value of the eigen value as computed
   * by an EigenExecutionerBase object.
   */
  virtual PostprocessorValue getValue() override;

private:
  /// Pointer to the attribute to report, this is assigned via the "value" parameter
  Real * _value;
};

#endif // EIGENVALUEREPORTER_H
