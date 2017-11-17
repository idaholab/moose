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
#ifndef BIHARMONIC_H
#define BIHARMONIC_H

#include "Kernel.h"

// Forward Declarations
class Biharmonic;

template <>
InputParameters validParams<Biharmonic>();

/**
 * Computes the residual and Jacobian contribution for the weak form
 * of the biharmonic equation:
 *
 * \int Laplacian(u) * Laplacian(v) dx
 */
class Biharmonic : public Kernel
{
public:
  Biharmonic(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const VariableSecond & _second_u;
  const VariablePhiSecond & _second_phi;
  const VariableTestSecond & _second_test;
};

#endif
