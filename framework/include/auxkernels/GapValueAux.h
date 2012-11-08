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

#ifndef GAPVALUEAUX_H
#define GAPVALUEAUX_H

#include "AuxKernel.h"
#include "PenetrationLocator.h"


//Forward Declarations
class GapValueAux;

template<>
InputParameters validParams<GapValueAux>();

class GapValueAux : public AuxKernel
{
public:
  GapValueAux(const std::string & name, InputParameters parameters);

  virtual ~GapValueAux();

protected:
  virtual Real computeValue();

  PenetrationLocator & _penetration_locator;

  const NumericVector<Number> * & _serialized_solution;

  DofMap & _dof_map;

  unsigned int _paired_variable;
  const bool _warnings;
};

#endif //GAPVALUEAUX_H
