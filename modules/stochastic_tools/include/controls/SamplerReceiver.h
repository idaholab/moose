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

#ifndef SAMPLERRECEIVER_H
#define SAMPLERRECEIVER_H

// MOOSE includes
#include "Control.h"

// Forward declarations
class SamplerReceiver;
class Function;

template <>
InputParameters validParams<SamplerReceiver>();

/**
 * A Control object for receiving data from a master application Sampler object.
 */
class SamplerReceiver : public Control
{
public:
  SamplerReceiver(const InputParameters & parameters);
  virtual void execute() override;

protected:

  /**
   * Update the parameter names and associated values.
   */
  void transfer(const std::vector<std::string> & names, const std::vector<Real> & values);

  /// Parameter names to modify
  std::vector<std::string> _parameters;

  /// Values to use when modifying parameters
  std::vector<Real> _values;

  /// Allows the SamplerTransfer to call the transfer method, which
  /// should only be called by that object so making it public is dangerous.
  friend class SamplerTransfer;
};

#endif
