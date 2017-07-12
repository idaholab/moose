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
   * Clears the list of parameters to modify
   */
  void reset();

  /**
   * Appends the list of parameters to modify
   */
  void addControlParameter(const std::string & name, const Real & value);

  /// Storage for the parameters to control
  std::map<std::string, Real> _parameters;

  /// Allows the SamplerTransfer to call the reset and addControlParameter methods, which
  /// should only be called by that object so making the public is dangerous.
  friend class SamplerTransfer;
};

#endif
