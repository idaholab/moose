//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADCOUPLEDTIMEDERIVATIVE_H
#define ADCOUPLEDTIMEDERIVATIVE_H

#include "ADKernelValue.h"

// Forward Declaration
template <ComputeStage>
class ADCoupledTimeDerivative;

declareADValidParams(ADCoupledTimeDerivative);

/**
 * This calculates the time derivative for a coupled variable
 **/
template <ComputeStage compute_stage>
class ADCoupledTimeDerivative : public ADKernelValue<compute_stage>
{
public:
  ADCoupledTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  const ADVariableValue & _v_dot;

  usingKernelValueMembers;
};

#endif // ADCOUPLEDTIMEDERIVATIVE_H
