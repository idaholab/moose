//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UzawaTransient.h"

#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "RigidBodyLoadControl.h"
#include "UzawaSolveObject.h"

#include "MooseObjectTagWarehouse.h"
#include "ScalarKernelBase.h"

registerMooseObject("ContactApp", UzawaTransient);

InputParameters
UzawaTransient::validParams()
{
  InputParameters params = Transient::validParams();
  params.addClassDescription("Transient executioner subclass that runs a Uzawa outer loop around "
                             "the standard per-time-step SNES solve for load-controlled rigid-body "
                             "contact.  See modules/contact/uzawa_solver_plan.md.");
  params.addParam<std::string>(
      "load_control_kernel",
      "",
      "Name of the [ScalarKernels/*] entry (a `type = RigidBodyLoadControl`) "
      "to toggle between ForceBalance and PinScalar modes between outer "
      "iterations.  Leave empty for displacement-controlled inputs -- the "
      "executioner then behaves identically to plain Transient.");
  params.addRangeCheckedParam<unsigned int>(
      "outer_max_iter",
      20,
      "outer_max_iter >= 1",
      "Cap on Uzawa outer iterations per time step.  On exhaustion the "
      "executioner reports `lastSolveConverged() = false` so "
      "IterationAdaptiveDT cuts back dt.");
  params.addRangeCheckedParam<Real>(
      "outer_abs_tol", 1e-8, "outer_abs_tol > 0", "Absolute tolerance on |R_s|.");
  params.addRangeCheckedParam<Real>(
      "outer_rel_tol",
      1e-6,
      "outer_rel_tol > 0",
      "Relative tolerance on |R_s|, measured against the value at outer iter 0.");
  params.addRangeCheckedParam<Real>(
      "max_step",
      std::numeric_limits<Real>::max(),
      "max_step > 0",
      "Trust-region clip on |ds| per outer iter.  Guards against a badly-"
      "chosen `kss_stiffness` producing a wild scalar step.");
  params.addRangeCheckedParam<unsigned int>(
      "damp_max_retries",
      0,
      "damp_max_retries >= 0",
      "Reserved for future use; not read by the current implementation.  "
      "The outer scalar Newton damps via `max_step`; if the primal solve "
      "itself fails, the executioner immediately propagates the failure so "
      "IterationAdaptiveDT cuts back.");
  params.addParam<bool>("outer_verbose",
                        true,
                        "Print one line per Uzawa outer iter to the console: `Uzawa outer iter "
                        "N: s = <value>, |R_s| = <value>`.  This is the load-controlled "
                        "solve's primary progress signal; on by default.  Inner primal "
                        "SNES/KSP output is unaffected by this flag -- MOOSE prints those "
                        "lines according to its usual PetscOutput conventions.");
  return params;
}

UzawaTransient::UzawaTransient(const InputParameters & parameters)
  : Transient(parameters),
    _load_control_kernel_name(getParam<std::string>("load_control_kernel")),
    _outer_max_iter(getParam<unsigned int>("outer_max_iter")),
    _outer_abs_tol(getParam<Real>("outer_abs_tol")),
    _outer_rel_tol(getParam<Real>("outer_rel_tol")),
    _max_step(getParam<Real>("max_step")),
    _damp_max_retries(getParam<unsigned int>("damp_max_retries")),
    _outer_verbose(getParam<bool>("outer_verbose"))
{
  // Construct the SolveObject inside the Executioner ctor so MooseObject's
  // "constructed via Factory" check sees `currentlyConstructing()` still
  // pointing at our params (same pattern PicardSolve/FixedPointSolve use).
  // The load-control handle is resolved later in init().
  _uzawa_solve = std::make_unique<UzawaSolveObject>(*this,
                                                    _outer_max_iter,
                                                    _outer_abs_tol,
                                                    _outer_rel_tol,
                                                    _max_step,
                                                    _damp_max_retries,
                                                    _outer_verbose);
}

UzawaTransient::~UzawaTransient() = default;

void
UzawaTransient::init()
{
  Transient::init();

  // Resolve the RigidBodyLoadControl kernel by name from the nonlinear
  // system's ScalarKernel warehouse.  If no name was supplied, or if the
  // named kernel isn't present, we run in pass-through mode.
  RigidBodyLoadControl * load_control = nullptr;
  if (!_load_control_kernel_name.empty())
  {
    auto & nl = _problem.getNonlinearSystemBase(0);
    const auto & sk_wh = nl.getScalarKernelWarehouse();
    const auto & sk_objs = sk_wh.getObjects();
    for (const auto & sk : sk_objs)
      if (sk->name() == _load_control_kernel_name)
      {
        load_control = dynamic_cast<RigidBodyLoadControl *>(sk.get());
        if (!load_control)
          mooseError("UzawaTransient: scalar kernel '",
                     _load_control_kernel_name,
                     "' is not a RigidBodyLoadControl (found type '",
                     sk->type(),
                     "').  Only RigidBodyLoadControl supports the mode/s_pin "
                     "toggling this executioner requires.");
        break;
      }
    if (!load_control)
      mooseError("UzawaTransient: no scalar kernel named '",
                 _load_control_kernel_name,
                 "' was found on nonlinear system 0.  Fix the "
                 "`load_control_kernel` parameter or remove it to fall back "
                 "to plain Transient behavior.");
  }

  // Wire the deferred references onto the already-constructed
  // SolveObject.  Inner-solve is the base Transient's own
  // `_fixed_point_solve` -- exactly what `timeStepSolveObject()` would
  // return by default.
  static_cast<UzawaSolveObject *>(_uzawa_solve.get())->setLoadControl(load_control);
  _uzawa_solve->setInnerSolve(*Transient::timeStepSolveObject());
}

SolveObject *
UzawaTransient::timeStepSolveObject()
{
  // Before init() has run, `_uzawa_solve` doesn't exist yet -- fall
  // back to the base's own inner solve object.  MOOSE calls this getter
  // during initialSetup which is AFTER our init(); this branch is
  // strictly defensive.
  if (!_uzawa_solve)
    return Transient::timeStepSolveObject();
  return _uzawa_solve.get();
}
