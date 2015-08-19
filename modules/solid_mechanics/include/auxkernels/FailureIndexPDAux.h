/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef FAILUREINDEXPDAUX_H
#define FAILUREINDEXPDAUX_H

#include "AuxKernel.h"
#include "FailureIndexPD.h"


//Forward Declarations
class FailureIndexPDAux;

template<>
InputParameters validParams<FailureIndexPDAux>();

class FailureIndexPDAux : public AuxKernel
{
public:

  FailureIndexPDAux(const InputParameters & parameters);

  virtual ~FailureIndexPDAux() {}

protected:

  const FailureIndexPD * const _failure_index_pd;
  virtual Real computeValue();

};

#endif //FAILUREINDEXPDAUX_H
