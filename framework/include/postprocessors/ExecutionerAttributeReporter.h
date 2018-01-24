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
