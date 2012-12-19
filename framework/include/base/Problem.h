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

#ifndef PROBLEM_H
#define PROBLEM_H

#include "ParallelUniqueId.h"
#include "InputParameters.h"
#include "ExecStore.h"
#include "MooseMesh.h"
#include "MooseArray.h"

// libMesh
#include "libmesh_common.h"
#include "equation_systems.h"
#include "quadrature.h"
#include "elem.h"
#include "node.h"
#include "nonlinear_implicit_system.h"

class MooseVariable;
class MooseVariableScalar;
class Output;
class Material;
class Function;
class TimePeriod;
class Problem;

template<>
InputParameters validParams<Problem>();

/**
 * Class that hold the whole problem being solved.
 */
class Problem : public MooseObject
{
public:
  Problem(const std::string & name, InputParameters parameters);
  virtual ~Problem();

  /**
   * Get the name of this problem
   * @return The name of this problem
   */
  virtual const std::string & name();

  /**
   * Get reference to all-purpose parameters
   */
  InputParameters & parameters() { return _pars; }

  virtual void init() = 0;

  // Output system /////
  virtual void output(bool force = false) = 0;
  void outputInitial(bool out_init);

  // Time periods //////

  /**
   * Add a time period
   * @param name Name of the time period
   * @param start_time Start time of the time period
   * @param end_time End time of the time period
   */
  TimePeriod & addTimePeriod(const std::string & name, Real start_time);

  /**
   * Get time period by name
   * @param name Name of the time period to get
   * @return Pointer to the time period struct if found, otherwise NULL
   */
  virtual TimePeriod * getTimePeriodByName(const std::string & name);

  const std::vector<TimePeriod *> & getTimePeriods() const;

protected:
  /// output initial condition if true
  bool _output_initial;

  /// Time periods
  std::vector<TimePeriod *> _time_periods;
};

#endif /* PROBLEM_H */
