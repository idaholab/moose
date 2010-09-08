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

#ifndef COUPLEDAUX_H
#define COUPLEDAUX_H

#include "AuxKernel.h"


//Forward Declarations
class CoupledAux;

template<>
InputParameters validParams<CoupledAux>();

/** 
 * Coupled auxiliary value
 */
class CoupledAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  CoupledAux(const std::string & name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~CoupledAux() {}
  
protected:
  virtual Real computeValue();

  Real _value;
  std::string _operator;

  int _coupled;
  VariableValue & _coupled_val;
};

#endif //COUPLEDAUX_H
