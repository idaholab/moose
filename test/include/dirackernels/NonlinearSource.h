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

#ifndef NONLINEARSOURCE_H
#define NONLINEARSOURCE_H

// Moose Includes
#include "DiracKernel.h"

// Forward Declarations
class NonlinearSource;

template <>
InputParameters validParams<NonlinearSource>();

/**
 * A DiracKernel with both on and off-diagonal Jacobian contributions
 * to test the off-diagonal contribution capability for Dirac kernels.
 */
class NonlinearSource : public DiracKernel
{
public:
  NonlinearSource(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

protected:
  // A coupled variable this kernel depends on
  const VariableValue & _coupled_var;
  unsigned _coupled_var_num;

  // A constant factor which controls the strength of the source.
  Real _scale_factor;
  Point _p;
};

#endif // NONLINEARSOURCE_H
