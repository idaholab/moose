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

#ifndef VARIABLEGRADIENTCOMPONENT_H
#define VARIABLEGRADIENTCOMPONENT_H

// MOOSE includes
#include "AuxKernel.h"

// Forward declarations
class VariableGradientComponent;

template<>
InputParameters validParams<VariableGradientComponent>();

/**
 * Extract a component from the gradient of a variable
 */
class VariableGradientComponent : public AuxKernel
{
public:

  /**
   * Class constructor
   * @param name AuxKernel object name
   * @param parameters Input parameters for the object
   */
  VariableGradientComponent(const std::string & name, InputParameters parameters);

protected:

  /**
   * Extracts the component from the gradient of a coupled variable
   */
  virtual Real computeValue();

private:

  /// Reference to the gradient of the coupled variable
  const VariableGradient & _gradient;

  /// Desired component
  int _component;
};

#endif // VARIABLEGRADIENTCOMPONENT_H
