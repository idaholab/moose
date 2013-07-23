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

#ifndef MULTIPLEUPDATEELEMAUX_H_
#define MULTIPLEUPDATEELEMAUX_H_

#include "AuxKernel.h"

class MultipleUpdateElemAux;

template<>
InputParameters validParams<MultipleUpdateElemAux>();

/**
 * Aux kernel that updated values of coupled variables
 */
class MultipleUpdateElemAux : public AuxKernel
{
public:
  MultipleUpdateElemAux(const std::string & name, InputParameters parameters);
  virtual ~MultipleUpdateElemAux();

protected:
  virtual void compute();
  virtual Real computeValue();
  virtual void computeVarValues(std::vector<Real> & values);

  unsigned int _n_vars;
  std::vector<MooseVariable *> _vars;
};

#endif /* MULTIPLEUPDATEELEMAUX_H_ */
