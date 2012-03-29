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

#ifndef SINNEUMANNBC_H
#define SINNEUMANNBC_H

#include "IntegratedBC.h"


//Forward Declarations
class SinNeumannBC;

template<>
InputParameters validParams<SinNeumannBC>();

/**
 * Implements a simple constant SinNeumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class SinNeumannBC : public IntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  SinNeumannBC(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

private:
  Real _initial;
  Real _final;
  Real _duration;

  /// Value of grad(u) on the boundary.
  Real _value;
};

#endif //SINNEUMANNBC_H
