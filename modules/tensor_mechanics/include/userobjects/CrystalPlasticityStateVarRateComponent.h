//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
