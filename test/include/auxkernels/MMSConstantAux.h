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

#ifndef MMSCONSTANTAUX_H
#define MMSCONSTANTAUX_H

#include "AuxKernel.h"

class MMSConstantAux;

template<>
InputParameters validParams<MMSConstantAux>();

class MMSConstantAux : public AuxKernel
{
public:

 MMSConstantAux(const std::string & name, InputParameters parameters);

  virtual ~MMSConstantAux() {}

protected:
  virtual Real computeValue();

  unsigned int _mesh_dimension;
};

#endif //MMSCONSTANTAUX_H
