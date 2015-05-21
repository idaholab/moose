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

#include "DIRK.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"

template<>
InputParameters validParams<DIRK>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

DIRK::DIRK(const std::string & name, InputParameters parameters) :
    TimeIntegrator(name, parameters),
    _stage(1),
    _residual_stage1(_nl.addVector("residual_stage1", false, GHOSTED)),
    _residual_stage2(_nl.addVector("residual_stage2", false, GHOSTED)),
    _solution_start(_sys.solutionOld())
{
}

DIRK::~DIRK()
{
}

void
DIRK::computeTimeDerivatives()
{
  
  _u_dot  = *_solution;
  
  if (_stage==1) {
    // Compute stage U_1
    _u_dot -= _solution_start;
    _u_dot *= 3. / _dt;
    _u_dot.close();
    
    _du_dot_du = 3. / _dt;
  }
  else if (_stage==2) {
    // Compute stage U_2
    _u_dot -= _solution_start;
    _u_dot *= 2. / _dt;
    _u_dot.close();
    
    _du_dot_du = 2. / _dt;
  }
  else if (_stage==3) {
    // Compute update
    _u_dot -= _solution_start;
    _u_dot *= 4. / _dt;
    _u_dot.close();
    
    _du_dot_du = 4. / _dt;
  }
  else {
    mooseError("DIRK::computeTimeDerivatives(): Member variable _stage can only have values 1, 2 or 3.");
  }
      
  _u_dot.close();
  
}


void
DIRK::solve() {
  
  // Time at end of step
  Real time = _fe_problem.time();
  
  // Time at beginning of step
  Real time_old = _fe_problem.timeOld();

  // Time at stage 1
  Real time_stage1 = time_old + (1./3.)*_dt;
  
  // Solution at beginning of time step; store it because it is needed in update step
  _solution_start = _solution_old;
  
  // Compute first stage
  _console << "DIRK: 1. stage" << std::endl;
  _stage = 1;
  _fe_problem.time() = time_stage1;
  _fe_problem.getNonlinearSystem().sys().solve();

  _fe_problem.initPetscOutput();
 
  // Compute second stage
  _console << "DIRK: 2. stage" << std::endl;
  _stage = 2;
  _fe_problem.timeOld() = time_stage1;
  _fe_problem.time()    = time;
  
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetOptions(_fe_problem);
#endif
  Moose::setSolverDefaults(_fe_problem);
  _fe_problem.getNonlinearSystem().sys().solve();

  _fe_problem.initPetscOutput();

  // Compute update
  _console << "DIRK: 3. stage" << std::endl;
  _stage = 3;
  
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetOptions(_fe_problem);
#endif
  Moose::setSolverDefaults(_fe_problem);
  _fe_problem.getNonlinearSystem().sys().solve();

  // Reset time at beginning of step to its original value
  _fe_problem.timeOld() = time_old;
  
}

void
DIRK::postStep(NumericVector<Number> & residual)
{
  
  if (_stage==1) {

    residual += _Re_time;
    residual += _Re_non_time;
    residual.close();

    _residual_stage1 = _Re_non_time;
    _residual_stage1.close();

  }
  else if (_stage==2) {
    
    residual += _Re_time;
    residual += _Re_non_time;
    residual += _residual_stage1;
    residual.close();
    
    _residual_stage2 = _Re_non_time;
    _residual_stage2.close();
  }
  else if (_stage==3) {
    residual = 0.0;
    residual += _residual_stage1;
    residual *= 3.;
    residual += _Re_time;
    residual += _residual_stage2;
    residual.close();
  }
  else {
    mooseError("DIRK::computeTimeDerivatives(): Member variable _stage can only have values 1, 2 or 3.");
  }
}