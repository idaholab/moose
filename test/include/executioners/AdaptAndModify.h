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

#ifndef ADAPTANDMODIFY_H
#define ADAPTANDMODIFY_H

#include "Transient.h"

// Forward Declarations
class AdaptAndModify;

template <>
InputParameters validParams<AdaptAndModify>();

class AdaptAndModify : public Transient
{
public:
  AdaptAndModify(const InputParameters & parameters);

  virtual void incrementStepOrReject();

  virtual void endStep(Real input_time = -1.0);

protected:
  unsigned int _adapt_cycles;
};

#endif // ADAPTANDMODIFY_H
