/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
