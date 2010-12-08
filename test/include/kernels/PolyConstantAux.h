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

#ifndef POLYCONSTANTAUX_H
#define POLYCONSTANTAUX_H

#include "AuxKernel.h"

class PolyConstantAux;

template<>
InputParameters validParams<PolyConstantAux>();

class PolyConstantAux : public AuxKernel
{
public:
  
 PolyConstantAux(const std::string & name, InputParameters parameters);

  virtual ~PolyConstantAux() {}
  
protected:
  virtual Real computeValue();
};

#endif //POLYCONSTANTAUX_H
