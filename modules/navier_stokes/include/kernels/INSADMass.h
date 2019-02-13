//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSADMASS_H
#define INSADMASS_H

#include "INSADBase.h"

// Forward Declarations
template <ComputeStage>
class INSADMass;

declareADValidParams(INSADMass);

/**
 * This class computes the mass equation residual and Jacobian
 * contributions (the latter using automatic differentiation) for the incompressible Navier-Stokes
 * equations.
 */
template <ComputeStage compute_stage>
class INSADMass : public ADKernelValue<compute_stage>
{
public:
  INSADMass(const InputParameters & parameters);

  virtual ~INSADMass() {}

protected:
  ADResidual precomputeQpResidual() override;

  /// The strong residual of the mass equation, computed using INSADMaterial
  const ADMaterialProperty(Real) & _mass_strong_residual;

  usingKernelValueMembers;
};

#endif // INSADMASS_H
