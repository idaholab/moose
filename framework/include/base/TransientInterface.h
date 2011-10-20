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


class SubProblemInterface;

/**
 * Interface for objects that needs transient capabilities
 *
 */
class TransientInterface
{
public:
  TransientInterface(InputParameters & parameters);
  virtual ~TransientInterface();

private:
  SubProblemInterface & _ti_subproblem;

protected:
  Real & _t;                                            ///< Time
  int & _t_step;                                        ///< The number of the time step
  Real & _dt;                                           ///< Time step size
  Real & _dt_old;                                       ///< Size of the old time step
  std::vector<Real> & _time_weight;                     ///< Time weights for time-stepping schemes (like BDF2, ...)

  // NOTE: dunno if it is set properly in time of instantiation (might be a source of bugs)
  bool _is_transient;
};

#endif /* TRANSIENTINTERFACE_H */
