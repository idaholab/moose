//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CRYSTALPLASTICITYSLIPRESISTANCE_H
#define CRYSTALPLASTICITYSLIPRESISTANCE_H

#include "CrystalPlasticityUOBase.h"

class CrystalPlasticitySlipResistance;

template <>
InputParameters validParams<CrystalPlasticitySlipResistance>();

/**
 * Crystal plasticity slip resistance userobject class.
 * The virtual functions written below must be
 * over-ridden in derived classes to provide actual values.
 */
class CrystalPlasticitySlipResistance : public CrystalPlasticityUOBase
{
public:
  CrystalPlasticitySlipResistance(const InputParameters & parameters);

  virtual bool calcSlipResistance(unsigned int qp, std::vector<Real> & val) const = 0;
};

#endif // CRYSTALPLASTICITYSLIPRESISTANCE_H
