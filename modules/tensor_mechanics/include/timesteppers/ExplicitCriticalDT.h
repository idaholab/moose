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

#ifndef EXPLICITCRITICALDT_H
#define EXPLICITCRITICALDT_H

#include "TimeStepper.h"

// Forward Declarations
class ExplicitCriticalDT;

template <>
InputParameters validParams<ExplicitCriticalDT>();

/**
 * This class cuts the timestep in half at every iteration
 * until it reaches a user-specified minimum value.
 */
class ExplicitCriticalDT : public TimeStepper
{
public:
  ExplicitCriticalDT(const InputParameters & parameters);

protected:
  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;
  Real _bulk_mod;
  Real _Emod;
  Real _vratio;
  Real _rho;
  Real _cwave;

private:
  Real _mesh_size;
  Real _min_dt;
};

#endif // ExplicitCriticalDT_H
