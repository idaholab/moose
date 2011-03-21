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

#ifndef CONSTANTAUX_H
#define CONSTANTAUX_H

#include "AuxKernel.h"


//Forward Declarations
class ConstantAux;

template<>
InputParameters validParams<ConstantAux>();

/** 
 * Constant auxiliary value
 */
class ConstantAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  ConstantAux(const std::string & name, InputParameters parameters);

  virtual ~ConstantAux() {}
  
protected:
  virtual Real computeValue();

  Real _value;
};

#endif //CONSTANTAUX_H
