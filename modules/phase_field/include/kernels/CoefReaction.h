/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COEFREACTION_H
#define COEFREACTION_H

#include "Reaction.h"

// Forward Declarations
class CoefReaction;

template <>
InputParameters validParams<CoefReaction>();

class CoefReaction : public Reaction
{
public:
  CoefReaction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// input parameter multiplied by the reaction kernel
  const Real _coef;
};

#endif // COEFREACTION_H
