/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACGRGRBASE_H
#define ACGRGRBASE_H

#include "ACBulk.h"

// Forward Declarations
class ACGrGrBase;

template <>
InputParameters validParams<ACGrGrBase>();

/**
 * This is the base class for kernels that calculate the residual for grain growth.
 * It calculates the residual of the ith order parameter, and the values of
 * all other order parameters are coupled variables and are stored in vals.
 */
class ACGrGrBase : public ACBulk<Real>
{
public:
  ACGrGrBase(const InputParameters & parameters);

protected:
  const unsigned int _op_num;

  std::vector<const VariableValue *> _vals;
  std::vector<unsigned int> _vals_var;

  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> & _tgrad_corr_mult;

  const VariableGradient * _grad_T;
};

#endif // ACGRGRBASE_H
