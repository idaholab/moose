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

#ifndef NODALNORMALCOMPONENTAUX_H
#define NODALNORMALCOMPONENTAUX_H

#include "AuxKernel.h"

class NodalNormalComponentAux;

template<>
InputParameters validParams<NodalNormalComponentAux>();

/**
 *
 */
class NodalNormalComponentAux : public AuxKernel
{
public:
  NodalNormalComponentAux(const std::string & name, InputParameters parameters);
  virtual ~NodalNormalComponentAux();

protected:
  virtual Real computeValue();

  NumericVector<Number> & _nx;
  NumericVector<Number> & _ny;
  NumericVector<Number> & _nz;

  unsigned int _component;
};

#endif /* NODALNORMALCOMPONENTAUX_H */
