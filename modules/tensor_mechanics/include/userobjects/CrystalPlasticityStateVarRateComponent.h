/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CRYSTALPLASTICITYSTATEVARRATECOMPONENT_H
#define CRYSTALPLASTICITYSTATEVARRATECOMPONENT_H

#include "CrystalPlasticityUOBase.h"

class CrystalPlasticityStateVarRateComponent;

template <>
InputParameters validParams<CrystalPlasticityStateVarRateComponent>();

/**
 * Crystal plasticity state variable evolution rate component userobject base class.
 * The virtual functions written below must be
 * over-ridden in derived classes to provide actual values
 */
class CrystalPlasticityStateVarRateComponent : public CrystalPlasticityUOBase
{
public:
  CrystalPlasticityStateVarRateComponent(const InputParameters & parameters);

  virtual bool calcStateVariableEvolutionRateComponent(unsigned int qp,
                                                       std::vector<Real> & val) const = 0;
};

#endif // CRYSTALPLASTICITYSTATEVARRATECOMPONENT_H
