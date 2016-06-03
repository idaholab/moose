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

#ifndef BADENUMAUX_H
#define BADENUMAUX_H

#include "AuxKernel.h"


//Forward Declarations
class BadEnumAux;

template<>
InputParameters validParams<BadEnumAux>();

/**
 * Self auxiliary value
 */
class BadEnumAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  BadEnumAux(const InputParameters & parameters);

  virtual ~BadEnumAux();

protected:
  virtual Real computeValue();
};

#endif //BADENUMAUX_H
