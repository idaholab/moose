//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once
#include "MFEMExecutioner.h"
#include "TimeDomainProblemOperator.h"

class MFEMTransient : public Executioner, public MFEMExecutioner
{
public:
  static InputParameters validParams();

  explicit MFEMTransient(const InputParameters & params);

  void constructProblemOperator() override;
  virtual void init() override;
  virtual void execute() override;

  virtual bool lastSolveConverged() const override { return true; };

  // virtual void computeDT();

  // Methods called before and after timestepping. Not used in MFEM timesteppers
  virtual void preStep(){};
  virtual void postStep(){};

  /**
   * Do whatever is necessary to advance one step.
   */
  virtual void takeStep(mfem::real_t input_dt = -1.0);

  /**
   * @return The the computed dt to use for this timestep.
   */
  virtual mfem::real_t getDT() const { return _dt; };

  /**
   * Transient loop will continue as long as this keeps returning true.
   */
  virtual bool keepGoing();

  /**
   * Can be used to set the next "target time" which is a time to nail perfectly.
   * Useful for driving MultiApps.
   */
  // virtual void setTargetTime(mfem::real_t target_time);

  /**
   * This is where the solve step is actually incremented.
   */
  virtual void incrementStepOrReject();

  virtual void endStep();

  /**
   * Get the current time.
   */
  virtual mfem::real_t getTime() const { return _time; };

  /**
   * Get the timestep tolerance
   * @return The timestep tolerance
   */
  mfem::real_t & timestepTol() { return _timestep_tolerance; }

  /**
   * Get a modifiable reference to the end time
   * @return The end time
   */
  mfem::real_t & endTime() { return _end_time; }

  mutable mfem::real_t _dt;     // Timestep size
  mutable mfem::real_t _dt_old; // Previous timestep size

private:
  mfem::real_t & _time;     // Current time
  mfem::real_t _start_time; // Start time
  mfem::real_t _end_time;   // End time
  mutable int _t_step;      // Current timestep index
  int _vis_steps;          // Number of cycles between each output update
  mutable bool _last_step; // Flag to check if current step is final
  std::unique_ptr<Moose::MFEM::TimeDomainProblemOperator> _problem_operator{nullptr};

  /**
   * Variables controlling termination:
   */
  mfem::real_t _timestep_tolerance;
  mfem::real_t _dtmin;
  mfem::real_t _dtmax;
  unsigned int _num_steps;
  bool _abort;
  /// This parameter controls how the system will deal with _dt <= _dtmin
  /// If true, the time stepper is expected to throw an error
  /// If false, the executioner will continue through EXEC_FINAL
  const bool _error_on_dtmin;
};

#endif
