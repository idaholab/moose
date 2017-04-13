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

#ifndef REALCONTROLPARAMETERREPORTER_H
#define REALCONTROLPARAMETERREPORTER_H

// MOOSE includes
#include "GeneralPostprocessor.h"

// Forward Declarations
class RealControlParameterReporter;

template <>
InputParameters validParams<RealControlParameterReporter>();

class RealControlParameterReporter : public GeneralPostprocessor
{
public:
  RealControlParameterReporter(const InputParameters & parameters);

  /**
   * Extract the parameter via the ControlInterface::getControlParam
   **/
  virtual void initialSetup();

  ///@{
  /**
   * These methods left intentionally empty
   */
  virtual void initialize() {}
  virtual void execute() {}
  ///@}

  /**
   * Return the parameter value
   */
  virtual Real getValue();

private:
  // Pointer to the parameter to report, a pointer is used because the access
  // of the parameter value must occur in initialSetup because all objects
  // must be created prior to attempting to access the parameter objects
  const Real * _parameter;
};

#endif // REALCONTROLPARAMETERREPORTER_H
