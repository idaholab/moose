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

#ifndef TRANSIENTINTERFACE_H
#define TRANSIENTINTERFACE_H

#include "InputParameters.h"


class Problem;

class TransientInterface
{
public:
  TransientInterface(InputParameters & parameters);
  virtual ~TransientInterface();

private:
  Problem & _ti_problem;

protected:
  Real & _t;
  int & _t_step;
  Real & _dt;
  Real & _dt_old;
  std::vector<Real> & _time_weight;

  // NOTE: dunno if it is set properly in time of instantiation (might be a source of bugs)
  bool _is_transient;
};

#endif /* TRANSIENTINTERFACE_H_ */
