//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SMALLSTRAINCONSTANTHORIZONBPD_H
#define SMALLSTRAINCONSTANTHORIZONBPD_H

#include "SmallStrainMaterialBaseBPD.h"

class SmallStrainConstantHorizonBPD;

template <>
InputParameters validParams<SmallStrainConstantHorizonBPD>();

/**
 * Material class for bond based peridynamic solid mechanics model based on regular spatial
 * discretization
 */
class SmallStrainConstantHorizonBPD : public SmallStrainMaterialBaseBPD
{
public:
  SmallStrainConstantHorizonBPD(const InputParameters & parameters);

protected:
  virtual void computePDMicroModuli() override;
};

#endif // SMALLSTRAINCONSTANTHORIZONBPD_H
