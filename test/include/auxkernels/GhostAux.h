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

#ifndef GHOSTAUX_H
#define GHOSTAUX_H

#include "AuxKernel.h"

//Forward Declarations
class GhostAux;
class GhostUserObject;

template<>
InputParameters validParams<GhostAux>();

/**
 * An AuxKernel that uses built-in Random number generation.
 */
class GhostAux : public AuxKernel
{
public:
  GhostAux(const InputParameters & params);

protected:
  virtual Real computeValue() override;

  const GhostUserObject & _ghost_uo;
};

#endif //GHOSTAUX_H
