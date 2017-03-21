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

class FEProblemBase;
class TransientInterface;

template <>
InputParameters validParams<TransientInterface>();

/**
 * Interface for objects that needs transient capabilities
 */
class TransientInterface
{
public:
  TransientInterface(const MooseObject * moose_object);
  virtual ~TransientInterface();

  bool isImplicit() { return _is_implicit; }

protected:
  const InputParameters & _ti_params;

  FEProblemBase & _ti_feproblem;

  /**
   * If the object is using implicit or explicit form. This does NOT mean time scheme,
   * but which values are going to be used in the object - either from current time or old time.
   * Note that
   * even explicit schemes have implicit form (it is the time derivative "kernel")
   */
  bool _is_implicit;

  /// Time
  Real & _t;

  /// The number of the time step
  int & _t_step;

  /// Time step size
  Real & _dt;

  /// Size of the old time step
  Real & _dt_old;

  // NOTE: dunno if it is set properly in time of instantiation (might be a source of bugs)
  bool _is_transient;

private:
  const std::string _ti_name;
};

#endif /* TRANSIENTINTERFACE_H */
