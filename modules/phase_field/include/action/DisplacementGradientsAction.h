/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DISPLACEMENTGRADIENTSACTION_H
#define DISPLACEMENTGRADIENTSACTION_H

#include "InputParameters.h"
#include "Action.h"

/**
 * Automatically generates all variables, Kernels, and Materials to ensure the
 * correct derivatives of the elastic free energy in a non-split Cahn-Hilliard
 * simulation are assembled.
 */
class DisplacementGradientsAction : public Action
{
public:
  DisplacementGradientsAction(const InputParameters & params);

  virtual void act();

private:
  std::vector<VariableName> _displacements;
  std::vector<VariableName> _displacement_gradients;
};

template <>
InputParameters validParams<DisplacementGradientsAction>();

#endif // DISPLACEMENTGRADIENTSACTION_H
