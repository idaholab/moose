/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PFFRACINTVAR_H
#define PFFRACINTVAR_H

#include "KernelValue.h"

//Forward Declarations
class PFFracIntVar;

template<>
InputParameters validParams<PFFracIntVar>();

/**
 * Phase-field fracture model
 * This class computes the residual and jacobian for the auxiliary variable beta
 * Refer to Formulation: Miehe et. al., Int. J. Num. Methods Engg., 2010, 83. 1273-1311 Equation 63
 */
class PFFracIntVar : public KernelValue
{
public:
  PFFracIntVar(const InputParameters & parameters);

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
};

#endif //PFFRACINTVAR_H
