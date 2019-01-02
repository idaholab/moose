//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef ADTHERMALCONDUCTIVITYTEST_H
#define ADTHERMALCONDUCTIVITYTEST_H

#include "ADMaterial.h"

// Forward Declarations
template <ComputeStage>
class ADThermalConductivityTest;

declareADValidParams(ADThermalConductivityTest);

template <ComputeStage compute_stage>
class ADThermalConductivityTest : public ADMaterial<compute_stage>
{
public:
  ADThermalConductivityTest(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  ADMaterialProperty(Real) & _diffusivity;
  const ADVariableValue & _temperature;
  const ADVariableValue & _c;

  usingMaterialMembers;
};

#endif // ADTHERMALCONDUCTIVITYTEST_H
