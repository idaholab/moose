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

#ifndef REALFUNCTIONCONTROL_H
#define REALFUNCTIONCONTROL_H

// MOOSE includes
#include "Control.h"

// Forward declarations
class RealFunctionControl;
class Function;

template <>
InputParameters validParams<RealFunctionControl>();

/**
 * A basic control for changing an input parameter using a Function
 */
class RealFunctionControl : public Control
{
public:
  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  RealFunctionControl(const InputParameters & parameters);

  virtual void execute() override;

private:
  /// The function to execute
  Function & _function;

  /// Vector of parameters to change
  ControllableParameter<Real> _parameters;
};

#endif // REALFUNCTIONCONTROL_H
