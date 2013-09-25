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

#ifndef UNIQUEIDAUX_H
#define UNIQUEIDAUX_H

#include "AuxKernel.h"


//Forward Declarations
class UniqueIDAux;

template<>
InputParameters validParams<UniqueIDAux>();

/**
 * UniqueID auxiliary value
 *
 */
class UniqueIDAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  UniqueIDAux(const std::string & name, InputParameters parameters);

  virtual ~UniqueIDAux();

protected:
  virtual Real computeValue();
};

#endif //UNIQUEIDAUX_H
