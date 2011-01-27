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

#ifndef COUPLEDGRADAUX_H
#define COUPLEDGRADAUX_H

#include "AuxKernel.h"


//Forward Declarations
class CoupledGradAux;

template<>
InputParameters validParams<CoupledGradAux>();

/** 
 * Coupled auxiliary value
 */
class CoupledGradAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  CoupledGradAux(const std::string & name, InputParameters parameters);

  virtual ~CoupledGradAux() {}
  
protected:
  virtual Real computeValue();

  RealGradient _grad;
  std::string _operator;

  int _coupled;
  VariableGradient & _coupled_grad;
};

#endif //COUPLEDGRADAUX_H
