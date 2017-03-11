/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef COARSENINGINTEGRALAUX_H
#define COARSENINGINTEGRALAUX_H

#include "AuxKernel.h"

class CoarseningIntegralTracker;
class CoarseningIntegralAux;

template<>
InputParameters validParams<CoarseningIntegralAux>();

/**
 * Corrective source term computed by the CoarseningIntegralTracker.
 */
class CoarseningIntegralAux : public AuxKernel
{
public:
  CoarseningIntegralAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const CoarseningIntegralTracker & _tracker;
};

#endif // COARSENINGINTEGRALAUX_H
