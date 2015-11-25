/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CRYSTALPLASTICITYSTATEVARIABLEEVOLUTIONRATECOMPONENT_H
#define CRYSTALPLASTICITYSTATEVARIABLEEVOLUTIONRATECOMPONENT_H

#include "CrystalPlasticityUOBase.h"

class CrystalPlasticityStateVariableEvolutionRateComponent;

template<>InputParameters validParams<CrystalPlasticityStateVariableEvolutionRateComponent>();

/**
 * Crystal plasticity state variable evolution rate component userobject base class.
 * The virtual functions written below must be
 * over-ridden in derived classes to provide actual values
 */
class CrystalPlasticityStateVariableEvolutionRateComponent : public CrystalPlasticityUOBase
{
 public:
   CrystalPlasticityStateVariableEvolutionRateComponent(const InputParameters & parameters);

   virtual bool calcStateVariableEvolutionRateComponent(unsigned int qp, std::vector<Real> & val) const = 0;
};

#endif // CRYSTALPLASTICITYSTATEVARIABLEEVOLUTIONRATECOMPONENT_H
