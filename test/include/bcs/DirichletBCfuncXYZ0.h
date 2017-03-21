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
#ifndef DIRICHLETBCFUNCXYZ0_H
#define DIRICHLETBCFUNCXYZ0_H

#include "NodalBC.h"
#include "UsrFunc.h"

// Forward Declarations
class DirichletBCfuncXYZ0;

template <>
InputParameters validParams<DirichletBCfuncXYZ0>();

/**
 * Implements space-dependent Dirichlet BC.
 */
class DirichletBCfuncXYZ0 : public NodalBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  DirichletBCfuncXYZ0(const InputParameters & parameters);

  /**
   * Destructor.
   */
  virtual ~DirichletBCfuncXYZ0() {}

protected:
  virtual Real computeQpResidual();

private:
  /**
   * Parameters for the manufactured solution used.
   */
  Real _A0;
  Real _B0;
  Real _C0;
  Real _omega0;
};

#endif // DIRICHLETBCFUNCXYZ0_H
