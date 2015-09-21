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

#include "LStableDirk3.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"

template<>
InputParameters validParams<LStableDirk3>()
{
  InputParameters params = validParams<TimeIntegrator>();
  return params;
}


LStableDirk3::LStableDirk3(const InputParameters & parameters) :
    TimeIntegrator(parameters),
    _stage(1),
    _residual_stage1(_nl.addVector("residual_stage1", false, GHOSTED)),
    _residual_stage2(_nl.addVector("residual_stage2", false, GHOSTED)),
    _residual_stage3(_nl.addVector("residual_stage3", false, GHOSTED)),
    _gamma(-std::sqrt(2.)*std::cos(atan(std::sqrt(2.)/4.)/3.)/2. + std::sqrt(6.)*std::sin(atan(std::sqrt(2.)/4.)/3.)/2. + 1.),
    _c1(_gamma),
    _c2(.5*(1+_gamma)),
    _c3(1.0),
    _a11(_gamma),
    _a21(.5*(1-_gamma)),
    _a22(_gamma),
    _a31(.25*(-6*_gamma*_gamma + 16*_gamma - 1)),
    _a32(.25*( 6*_gamma*_gamma - 20*_gamma + 5)),
    _a33(_gamma)
{
}



LStableDirk3::~LStableDirk3()
{
}



void
LStableDirk3::computeTimeDerivatives()
{
  // We are multiplying by the method coefficients in postStep(), so
  // the time derivatives are of the same form at every stage although
  // the current solution varies depending on the stage.
  _u_dot  = *_solution;
  _u_dot -= _solution_old;
  _u_dot *= 1. / _dt;
  _u_dot.close();
  _du_dot_du = 1. / _dt;
}



void
LStableDirk3::solve()
{
  // Time at end of step
  Real time_old = _fe_problem.timeOld();

  // Compute first stage
  _console << "1st stage\n";
  _stage = 1;
  _fe_problem.time() = time_old + _c1*_dt;
  _fe_problem.getNonlinearSystem().sys().solve();



  // Compute second stage
  _fe_problem.initPetscOutput();
  _console << "2nd stage\n";
  _stage = 2;
  _fe_problem.time() = time_old + _c2*_dt;

#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetOptions(_fe_problem);
#endif
  Moose::setSolverDefaults(_fe_problem);
  _fe_problem.getNonlinearSystem().sys().solve();



  // Compute third stage
  _fe_problem.initPetscOutput();
  _console << "3rd stage\n";
  _stage = 3;
  _fe_problem.time() = time_old + _c3*_dt;

#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetOptions(_fe_problem);
#endif
  Moose::setSolverDefaults(_fe_problem);
  _fe_problem.getNonlinearSystem().sys().solve();
}



void
LStableDirk3::postStep(NumericVector<Number> & residual)
{
  // In the standard RK notation, the residual of stage 1 of s is given by:
  //
  // R := M*(Y_i - y_n)/dt - \sum_{j=1}^s a_{ij} * f(t_n + c_j*dt, Y_j) = 0
  //
  // where:
  // .) M is the mass matrix
  // .) Y_i is the stage solution
  // .) dt is the timestep, and is accounted for in the _Re_time residual.
  // .) f are the "non-time" residuals evaluated for a given stage solution.
  if (_stage == 1)
  {
    _residual_stage1 = _Re_non_time;
    _residual_stage1.close();

    residual.add(1., _Re_time);
    residual.add(_a11, _residual_stage1);
    residual.close();
  }
  else if (_stage == 2)
  {
    _residual_stage2 = _Re_non_time;
    _residual_stage2.close();

    residual.add(1., _Re_time);
    residual.add(_a21, _residual_stage1);
    residual.add(_a22, _residual_stage2);
    residual.close();
  }
  else if (_stage == 3)
  {
    _residual_stage3 = _Re_non_time;
    _residual_stage3.close();

    residual.add(1., _Re_time);
    residual.add(_a31, _residual_stage1);
    residual.add(_a32, _residual_stage2);
    residual.add(_a33, _residual_stage3);
    residual.close();
  }

  else
    mooseError("LStableDirk3::postStep(): Member variable _stage can only have values 1, 2, or 3.");
}
