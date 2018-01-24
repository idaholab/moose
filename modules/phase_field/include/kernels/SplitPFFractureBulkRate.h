/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
