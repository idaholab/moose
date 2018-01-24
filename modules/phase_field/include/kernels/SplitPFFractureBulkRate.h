//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPLITPFFRACTUREBULKRATE_H
#define SPLITPFFRACTUREBULKRATE_H

#include "PFFractureBulkRateBase.h"

// Forward Declarations
class SplitPFFractureBulkRate;

template <>
InputParameters validParams<SplitPFFractureBulkRate>();

/**
 * Phase field based fracture model, split form where beta = laplace(c)
 * This kernel computes the residual and Jacobian for bulk free energy contribution to c
 * Refer to Formulation: Miehe et. al., Int. J. Num. Methods Engg., 2010, 83. 1273-1311 Equation 63
 */
class SplitPFFractureBulkRate : public PFFractureBulkRateBase
{
public:
  SplitPFFractureBulkRate(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Auxiliary variable: beta = Laplacian of c
  const VariableValue & _beta;
  const unsigned int _beta_var;
};

#endif // SPLITPFFRACTUREBULKRATE_H
