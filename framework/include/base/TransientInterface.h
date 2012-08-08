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


class FEProblem;
class TimePeriod;
class TransientInterface;

template<>
InputParameters validParams<TransientInterface>();

/**
 * Interface for objects that needs transient capabilities
 *
 */
class TransientInterface
{
public:
  TransientInterface(InputParameters & parameters, const std::string & name, const std::string & object_type);
  virtual ~TransientInterface();

  /**
   * Is the object active at the given time?
   * @return true is it is active, otherwise false
   */
  virtual bool isActive();

private:
  FEProblem & _ti_feproblem;

protected:
  /// If the object is using implicit or explicit form. This does NOT mean time scheme, but which values are going to be used in the object - either from current time or old time. Note that
  /// even explicit schemes have implicit form (it is the time derivative "kernel")
  bool _is_implicit;
  /// Time
  Real & _t;
  /// The number of the time step
  int & _t_step;
  /// Time step size
  Real & _dt;
  /// Size of the old time step
  Real & _dt_old;
  /// Time weights for time-stepping schemes (like BDF2, ...)
  std::vector<Real> & _time_weight;

  // NOTE: dunno if it is set properly in time of instantiation (might be a source of bugs)
  bool _is_transient;

  const std::string _object_type;

  const std::vector<TimePeriod *> & _time_periods;

private:
  const std::string _ti_name;
};

#endif /* TRANSIENTINTERFACE_H */
