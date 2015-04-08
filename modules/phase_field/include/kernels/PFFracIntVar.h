/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef PFFRACINTVAR_H
#define PFFRACINTVAR_H

#include "KernelValue.h"

/**
 * Phase-field fracture model
 * This class computes the residual and jacobian for the auxiliary variable beta
 * Refer to Formulation: Miehe et. al., Int. J. Num. Methods Engg., 2010, 83. 1273-1311 Equation 63
 */

//Forward Declarations
class PFFracIntVar;

template<>
InputParameters validParams<PFFracIntVar>();

class PFFracIntVar : public KernelValue
{
public:
  PFFracIntVar(const std::string & name, InputParameters parameters);

protected:

  enum PFFunctionType
  {
    Jacobian,
    Residual
  };

  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
};

#endif //PFFRACINTVAR_H
