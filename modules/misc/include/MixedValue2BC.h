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

#ifndef MIXEDVALUE2BC_H
#define MIXEDVALUE2BC_H

#include "IntegratedBC.h"


class MixedValue2BC;

template<>
InputParameters validParams<MixedValue2BC>();

class MixedValue2BC : public IntegratedBC
{
public:
  MixedValue2BC(const std::string & name, InputParameters parameters);


protected:
  virtual Real computeQpResidual();
};


#endif //MIXEDVALUE2BC_H
