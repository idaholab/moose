/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYSTOREDENERGYAUX_H
#define POLYSTOREDENERGYAUX_H

#include "AuxKernel.h"

//Forward Declarations
class PolyStoredEnergyAux;
class GrainTrackerInterface;

template<>
InputParameters validParams<PolyStoredEnergyAux>();

class PolyStoredEnergyAux : public AuxKernel
{
public:
  PolyStoredEnergyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  unsigned int _ncrys;
  std::vector<VariableValue *> _vals;
  std::vector<Real> _stored_energy;
  const GrainTrackerInterface & _grain_tracker;
};

#endif //POLYSTOREDENERGYAUX_H
