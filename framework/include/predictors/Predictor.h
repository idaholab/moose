//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseObject.h"
#include "Restartable.h"
#include "MooseObjectWarehouse.h"

class FEProblemBase;
class NonlinearSystemBase;
class Damper;
class ElementDamper;
class NodalDamper;
class GeneralDamper;

namespace libMesh
{
template <typename T>
class NumericVector;
}

/**
 * Base class for predictors.
 */
class Predictor : public MooseObject, public Restartable
{
public:
  static InputParameters validParams();

  Predictor(const InputParameters & parameters);
  virtual ~Predictor();

  virtual int order() { return 0; }
  virtual void initialSetup();
  virtual void timestepSetup();
  virtual bool shouldApply();
  virtual void apply(NumericVector<Number> & sln) = 0;

  /// Apply the prediction to \p sln, with its change scaled by the dampers of this predictor
  void applyDamped(NumericVector<Number> & sln);

  /// Whether \p name is one of the dampers of this predictor
  bool usesDamper(const std::string & name) const;

  /// Add \p damper (one per thread) to the dampers of this predictor
  void addDamper(const std::shared_ptr<Damper> & damper, THREAD_ID tid);

  /// Update the active dampers of this predictor, e.g. after a Control changed their enable
  void updateActive(THREAD_ID tid);

  virtual NumericVector<Number> & solutionPredictor() { return _solution_predictor; }

protected:
  FEProblemBase & _fe_problem;
  NonlinearSystemBase & _nl;

  int & _t_step;
  Real & _dt;
  Real & _dt_old;
  const NumericVector<Number> & _solution;
  NumericVector<Number> & _solution_old;
  NumericVector<Number> & _solution_older;
  NumericVector<Number> & _solution_predictor;
  int & _t_step_old;
  bool & _is_repeated_timestep;

  /// Whether to apply the prediction (controllable)
  const bool & _apply;

  /// Amount by which to scale the predicted value.  Must be in [0,1].
  Real _scale;

  /// Times for which the predictor should not be applied
  std::vector<Real> _skip_times;

  /// Old times for which the predictor should not be applied
  std::vector<Real> _skip_times_old;

  /// Option to skip prediction after a failed timestep
  const bool & _skip_after_failed_timetep;

  /// Timestep tolerance from Transient executioner
  const Real & _timestep_tolerance;

  /// Names of the dampers that limit the change of the prediction
  const std::vector<std::string> & _damper_names;

  /// The dampers of this predictor, which do not damp the nonlinear updates
  ///@{
  MooseObjectWarehouse<ElementDamper> _element_dampers;
  MooseObjectWarehouse<NodalDamper> _nodal_dampers;
  MooseObjectWarehouse<GeneralDamper> _general_dampers;
  ///@}

  /// Names of the dampers added to this predictor
  std::set<std::string> _added_dampers;

  /// Whether a failed damper cuts the time step instead of skipping the prediction
  const bool _cut_timestep_on_damping_failure;

  friend class SetupPredictorAction;
};
