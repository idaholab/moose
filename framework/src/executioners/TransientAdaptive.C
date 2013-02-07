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

#include "TransientAdaptive.h"

//Moose includes
#include "Kernel.h"
#include "Factory.h"
#include "SubProblem.h"
#include "Conversion.h"
#include "FEProblem.h"

//libMesh includes
#include "libmesh/implicit_system.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/transient_system.h"
#include "libmesh/petsc_vector.h"

// C++ Includes
#include <iomanip>
#include <iostream>
#include <fstream>

#if defined(LIBMESH_HAVE_PETSC) && defined(PETSC_VERSION_LE)
#if !PETSC_VERSION_LE(3,3,0)

template<>
InputParameters validParams<TransientAdaptive>()
{
  MooseEnum schemes("implicit-euler, explicit-euler, crank-nicolson, bdf2, petsc", "implicit-euler");
  InputParameters params = validParams<Executioner>();
  std::vector<Real> sync_times(1);
  sync_times[0] = -1;

  params.addParam<Real>("start_time",      0.0,    "The start time of the simulation");
  params.addParam<Real>("end_time",        1.0e30, "The end time of the simulation");
  params.addRequiredParam<Real>("dt", "The timestep size between solves");
  params.addParam<Real>("dtmin",           0.0,    "The minimum timestep size in an adaptive run");
  params.addParam<Real>("dtmax",           1.0e30, "The maximum timestep size in an adaptive run");
  params.addParam<int>("num_steps",        std::numeric_limits<int>::max(),     "The number of timesteps in a transient run");

  params.addParam<std::vector<std::string> >("time_periods", "The names of periods");
  params.addParam<std::vector<Real> >("time_period_starts", "The start times of time periods");
  params.addParam<std::vector<Real> >("time_period_ends", "The end times of time periods");
  params.addParam<MooseEnum>("scheme", schemes, "Time integration scheme used.");
  return params;
}

/* This simple class has a reduced interface */
class TimeStepper {
protected:
  TimeStepper(FEProblem &fep) : _fe_problem(fep), _laststep(-1), _u_ghosted(NULL), _udot_ghosted(NULL) {};
public:
  virtual ~TimeStepper() {};
  virtual void setInitialTimeStep(Real start_time,Real dt) = 0;
  virtual void setDuration(int maxsteps,Real final_time) = 0;
  virtual void setStepLimits(Real dtmin,Real dtmax) = 0;
  virtual void setupInternal(NumericVector<Number>&) = 0;
  /// Takes one step, calling preStep and preStage
  virtual TimeStepperStatus step(Real *endtime) = 0;
  /// Callable after step() completes, providing state at a time during the last step
  virtual void interpolate(Real time,NumericVector<Number> &X) = 0;
public:
    void setup(NumericVector<Number> &u_initial) {
    // work vectors
    _u_ghosted = &_fe_problem.getNonlinearSystem().sys().add_vector("TimeStepperWorkUGhosted", false, GHOSTED);
    _udot_ghosted = &_fe_problem.getNonlinearSystem().sys().add_vector("TimeStepperWorkUDotGhosted", false, GHOSTED);
    setupInternal(u_initial);
  }
protected:
  FEProblem &_fe_problem;
  int _laststep;
  NumericVector<Number> *_u_ghosted, *_udot_ghosted;
  void setTime(Real time,int step,Real dt) {
    _fe_problem.time() = time;
    _fe_problem.timeStep() = step;
    _fe_problem.dt() = dt;
  }
  void computeTransientImplicitResidual(Real time,NumericVector<Number> &u,NumericVector<Number> &udot,NumericVector<Number> &residual)
  {
    *_u_ghosted = u;
    *_udot_ghosted = udot;
    _fe_problem.computeTransientImplicitResidual(time,*_u_ghosted,*_udot_ghosted,residual);
  }
  void computeTransientImplicitJacobian(Real time,NumericVector<Number> &u,NumericVector<Number> &udot,Real shift,SparseMatrix<Number> &jacobian)
  {
    // The API currently guarantees that the residual has been called before the Jacobian, so we skip this update
    if (0) {
      *_u_ghosted = u;
      *_udot_ghosted = udot;
    }
    _fe_problem.computeTransientImplicitJacobian(time,*_u_ghosted,*_udot_ghosted,shift,jacobian);
  }
  void preStep(Real time,int step,Real dt) {
    setTime(time,step,dt);
    _fe_problem.onTimestepBegin();
    if (step != _laststep) _fe_problem.updateMaterials();
    _laststep = step;
  }
  void preStage(Real stagetime,int step,Real dt) {
    setTime(stagetime,step,dt);
    // Compute TimestepBegin AuxKernels
    _fe_problem.computeAuxiliaryKernels(EXEC_TIMESTEP_BEGIN);
    // Compute the Error Indicators
    _fe_problem.computeIndicators();
    // Compute TimestepBegin Postprocessors
    _fe_problem.computeUserObjects(EXEC_TIMESTEP_BEGIN);
  }
};

class PetscTimeStepper : public TimeStepper {
private:
  friend TimeStepper *createTimeStepper(Moose::TimeSteppingScheme, FEProblem&);
  PetscTimeStepper(FEProblem &feproblem) : TimeStepper(feproblem) {
    PetscErrorCode ierr;
    ierr = TSCreate(libMesh::COMM_WORLD, &this->_ts);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    ierr = TSSetApplicationContext(this->_ts,this);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  };
  virtual ~PetscTimeStepper() {
    // Intentionally do not check error code because we should not throw exceptions from the destructor
    TSDestroy(&this->_ts);
  };
  virtual void setInitialTimeStep(Real start_time,Real dt) {
    PetscErrorCode ierr;
    ierr = TSSetInitialTimeStep(this->_ts,start_time,dt);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
  virtual void setDuration(int max_steps,Real final_time) {
    PetscErrorCode ierr;
    ierr = TSSetDuration(this->_ts,max_steps,final_time);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
  virtual void setStepLimits(Real dtmin,Real dtmax) {
    PetscErrorCode ierr;
    TSAdapt adapt;
    ierr = TSGetAdapt(this->_ts,&adapt);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    ierr = TSAdaptSetStepLimits(adapt,dtmin,dtmax);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
  virtual void setupInternal(NumericVector<Number> &X) {
    PetscVector<Number> *pX = libmesh_cast_ptr<PetscVector<Number> *>(&X);
    PetscErrorCode ierr;
    ierr = TSSetIFunction(this->_ts,PETSC_NULL,this->_computeIFunction,this);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    PetscMatrix<Number> *mat = libmesh_cast_ptr<PetscMatrix<Number> *>(this->_fe_problem.getNonlinearSystem().sys().matrix);
    Mat pmat = mat->mat();
    ierr = TSSetIJacobian(this->_ts,pmat,pmat,this->_computeIJacobian,this);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    ierr = TSSetFromOptions(this->_ts);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    ierr = TSSetSolution(_ts,pX->vec());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
  virtual TimeStepperStatus step(Real *ftime) {
    PetscErrorCode ierr;
    TSConvergedReason reason;
    ierr = TSStep(_ts);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    ierr = TSGetConvergedReason(_ts,&reason);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    ierr = TSGetTime(_ts,ftime);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    return (TimeStepperStatus)reason;
  }
  virtual void interpolate(Real time,NumericVector<Number> &X) {
    PetscVector<Number> *pX = libmesh_cast_ptr<PetscVector<Number> *>(&X);
    PetscErrorCode ierr;
    ierr = TSInterpolate(this->_ts,time,pX->vec());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
  // Implementation of callbacks from PETSc
  static PetscErrorCode _computeIFunction(TS /* ts */,PetscReal time,Vec X,Vec Xdot,Vec F,void *ctx) {
    PetscTimeStepper *ths = (PetscTimeStepper*)ctx;
    PetscVector<Number> mX(X), mXdot(Xdot), mF(F);
    ths->computeTransientImplicitResidual(time,mX,mXdot,mF);
    return 0;
  }
  static PetscErrorCode _computeIJacobian(TS /* ts */,PetscReal time,Vec X,Vec Xdot,PetscReal shift,Mat *J,Mat *B,MatStructure *mstr,void *ctx) {
    PetscTimeStepper *ths = (PetscTimeStepper*)ctx;
    PetscVector<Number> mX(X), mXdot(Xdot);
    PetscMatrix<Number> mJ(*J), mB(*B);
    ths->computeTransientImplicitJacobian(time,mX,mXdot,shift,mB);
    mJ.close();
    *mstr = SAME_NONZERO_PATTERN;
    return 0;
  }
  static PetscErrorCode _preStep(TS ts) {
    PetscTimeStepper *ths;
    PetscErrorCode ierr;
    PetscReal time,stepsize;
    int step;
    ierr = TSGetApplicationContext(ts,&ths);CHKERRQ(ierr);
    ierr = TSGetTime(ts,&time);CHKERRQ(ierr);
    ierr = TSGetTimeStep(ts,&stepsize);CHKERRQ(ierr);
    ierr = TSGetTimeStepNumber(ts,&step);CHKERRQ(ierr);
    ths->preStep(time,step+1,stepsize);
    return 0;
  }
  static PetscErrorCode _preStage(TS ts, PetscReal stage_time) {
    PetscTimeStepper *ths;
    PetscErrorCode ierr;
    PetscReal stepsize;
    int step;
    ierr = TSGetApplicationContext(ts,&ths);CHKERRQ(ierr);
    ierr = TSGetTimeStep(ts,&stepsize);CHKERRQ(ierr);
    ierr = TSGetTimeStepNumber(ts,&step);CHKERRQ(ierr);
    ths->preStage(stage_time,step+1,stepsize);
    return 0;
  }

  TS _ts;
};

TimeStepper *createTimeStepper(Moose::TimeSteppingScheme scheme, FEProblem &fe_problem)
{
  switch (scheme) {
  case Moose::PETSC_TS:
    return new PetscTimeStepper(fe_problem);
    break;
  default:
    mooseError("Time stepping scheme not supported");
  }
}

TransientAdaptive::TransientAdaptive(const std::string & name, InputParameters parameters) :
  Executioner(name, parameters),
  _fe_problem(*getParam<FEProblem *>("_fe_problem")),
  _time_stepper(NULL)
{
  if (!_restart_file_base.empty())
    _fe_problem.setRestartFile(_restart_file_base);
  _fe_problem.transient(true);

  // Configure time stepper
  Moose::TimeSteppingScheme scheme(Moose::stringToEnum<Moose::TimeSteppingScheme>(getParam<MooseEnum>("scheme")));
  _time_stepper = createTimeStepper(scheme, _fe_problem);
  Real start_time = getParam<Real>("start_time");
  Real dt = getParam<Real>("dt");
  _time_stepper->setInitialTimeStep(start_time,dt);
  Real end_time = getParam<Real>("end_time");
  int num_steps = getParam<int>("num_steps");
  _time_stepper->setDuration(num_steps,end_time);
  Real dtmin = getParam<Real>("dtmin");
  Real dtmax = getParam<Real>("dtmax");
  _time_stepper->setStepLimits(dtmin,dtmax);

  // What should we do with these? I would prefer that they be removed from FEProblem.
#if 0
  Real time = _fe_problem.time();
  Real t_step(_fe_problem.timeStep());
#endif
}

TransientAdaptive::~TransientAdaptive()
{
  // This problem was built by the Factory and needs to be released by this destructor
  delete &_fe_problem;
  delete _time_stepper;
}

Problem &
TransientAdaptive::problem()
{
  return _fe_problem;
}

bool
TransientAdaptive::keepGoing(TimeStepperStatus status,Real /* time */) const
{
  // This is only responsible for checking for conditions other than number of steps and final time
  if (status != STATUS_ITERATING) return false;
  return true;
}

void
TransientAdaptive::execute()
{
  TimeStepperStatus status = STATUS_ITERATING;
  Real ftime = -1e100;
  _fe_problem.initialSetup();
  _time_stepper->setup(*_fe_problem.getNonlinearSystem().sys().solution);

  preExecute();

  // Start time loop...
  while (keepGoing(status,ftime))
  {
    status = _time_stepper->step(&ftime);
    _fe_problem.getNonlinearSystem().update();
    _fe_problem.getNonlinearSystem().set_solution(*_fe_problem.getNonlinearSystem().sys().current_local_solution);
    postSolve();
    _fe_problem.onTimestepEnd();
    _fe_problem.computeUserObjects();

    bool reset_dt = false;      /* has some meaning in Transient::computeConstrainedDT, but we do not use that logic here */
    _fe_problem.output(reset_dt);
    _fe_problem.outputPostprocessors(reset_dt);

#ifdef LIBMESH_ENABLE_AMR
    if (_fe_problem.adaptivity().isOn())
    {
      _fe_problem.adaptMesh();
      _fe_problem.out().meshChanged();
    }
#endif
  }
  postExecute();
}

#endif
#endif
