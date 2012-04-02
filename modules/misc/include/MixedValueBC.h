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

#ifndef MIXEDVALUEBC_H
#define MIXEDVALUEBC_H

#include "IntegratedBC.h"


class MixedValueBC;

template<>
InputParameters validParams<MixedValueBC>();

class MixedValueBC : public IntegratedBC
{
public:
  MixedValueBC(const std::string & name, InputParameters parameters);


protected:
  virtual Real computeQpResidual();
  
  Real _value;
};


#endif //MIXEDVALUEBC_H
