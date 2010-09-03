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

#ifndef PENETRATIONAUX_H
#define PENETRATIONAUX_H

#include "AuxKernel.h"
#include "PenetrationLocator.h"


//Forward Declarations
class PenetrationAux;

template<>
InputParameters validParams<PenetrationAux>();

/** 
 * Constant auxiliary value
 */
class PenetrationAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  PenetrationAux(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~PenetrationAux() {}

  virtual void setup();
  
protected:
  virtual Real computeValue();

  PenetrationLocator _penetration_locator;
};

#endif //PENETRATIONAUX_H
