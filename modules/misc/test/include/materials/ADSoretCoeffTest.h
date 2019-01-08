//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef ADSORETCOEFFTEST_H
#define ADSORETCOEFFTEST_H

#include "ADMaterial.h"

template <ComputeStage>
class ADSoretCoeffTest;

declareADValidParams(ADSoretCoeffTest);

template <ComputeStage compute_stage>
class ADSoretCoeffTest : public ADMaterial<compute_stage>
{
public:
  ADSoretCoeffTest(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const ADVariableValue & _coupled_var;
  const ADVariableValue & _temp;

  ADMaterialProperty(Real) & _soret_coeff;

  usingMaterialMembers;
};

#endif // ADSORETCOEFFTEST_H
