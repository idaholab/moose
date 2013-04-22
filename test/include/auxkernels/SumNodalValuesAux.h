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

#ifndef SUMNODALVALUESAUX_H
#define SUMNODALVALUESAUX_H

#include "AuxNodalScalarKernel.h"

class SumNodalValuesAux;

template<>
InputParameters validParams<SumNodalValuesAux>();

/**
 *
 */
class SumNodalValuesAux : public AuxNodalScalarKernel
{
public:
  SumNodalValuesAux(const std::string & name, InputParameters parameters);
  virtual ~SumNodalValuesAux();

  virtual void compute();

protected:
  virtual Real computeValue();

  VariableValue & _sum_var;
};

#endif /* SUMNODALVALUESAUX_H_ */
