/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EULERANGLE2RGBACTION_H
#define EULERANGLE2RGBACTION_H

#include "InputParameters.h"
#include "Action.h"

/**
 * Automatically generates all variables, Kernels, and Materials to ensure the
 * correct derivatives of the elastic free energy in a non-split Cahn-Hilliard
 * simulation are assembled.
 */
class EulerAngle2RGBAction : public Action
{
public:
  EulerAngle2RGBAction(const InputParameters & params);

  virtual void act();

private:
  const std::string _var_name_base;
};

template <>
InputParameters validParams<EulerAngle2RGBAction>();

#endif // EULERANGLE2RGBACTION_H
