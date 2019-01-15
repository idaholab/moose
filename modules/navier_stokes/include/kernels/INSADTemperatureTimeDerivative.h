//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSADTEMPERATURETIMEDERIVATIVE_H
#define INSADTEMPERATURETIMEDERIVATIVE_H

#include "ADTimeKernel.h"

// Forward Declarations
template <ComputeStage compute_stage>
class INSADTemperatureTimeDerivative;

declareADValidParams(INSADTemperatureTimeDerivative);

/**
 * This class computes the time derivative for the incompressible
 * Navier-Stokes momentum equation.  Could instead use CoefTimeDerivative
 * for this.
 */
template <ComputeStage compute_stage>
class INSADTemperatureTimeDerivative : public ADTimeKernel<compute_stage>
{
public:
  INSADTemperatureTimeDerivative(const InputParameters & parameters);

  virtual ~INSADTemperatureTimeDerivative() {}

protected:
  virtual ADResidual computeQpResidual();

  const ADMaterialProperty(Real) & _rho;
  const ADMaterialProperty(Real) & _cp;

  usingTimeKernelMembers;
};

#endif // INSADTEMPERATURETIMEDERIVATIVE_H
