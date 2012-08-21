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

#include "TimeScheme.h"
#include "NonlinearSystem.h"
#include "PetscSupport.h"
#include "FEProblem.h"

// libMesh
#include "nonlinear_solver.h"
#include "quadrature_gauss.h"
#include "dense_vector.h"
#include "boundary_info.h"
#include "petsc_matrix.h"
#include "petsc_vector.h"
#include "petsc_nonlinear_solver.h"
#include "numeric_vector.h"
#include "mesh.h"
#include "dense_subvector.h"
#include "dense_submatrix.h"
#include "dof_map.h"
// PETSc
#ifdef LIBMESH_HAVE_PETSC
#include "petscsnes.h"
#endif

#include <iostream>



TimeScheme::TimeScheme(NonlinearSystem * c) :
_use_AB2(false),
_use_littlef(false),
_nl(c),
_solution_u_dot(c->_sys.add_vector("u_dot", false, GHOSTED)),
_solution_du_dot_du(c->_sys.add_vector("du_dot_du", false, GHOSTED)),
_residual_old( c->_sys.add_vector("residual_old", false, GHOSTED)),
_predicted_solution(c->_sys.add_vector("predicted_solution", false, GHOSTED)),
_trash1(c->_sys.add_vector("trash1", false, GHOSTED)),
_trash2(c->_sys.add_vector("trash2", false, GHOSTED)),
_trash3(c->_sys.add_vector("trash3", false, GHOSTED)),
_mmatrix(c->_sys.add_vector("mmatrix", false, GHOSTED)),
_dt(c->_dt),
_dt_old( c->_dt_old),
_time_weight( c->_time_weight),
_time_stepping_scheme( c->_time_stepping_scheme),
_t_step(c->_t_step),
_time_stack(std::deque<TimeStep>()),
_workvecs(std::vector<NumericVector<Number> *>()),
_apply_predictor(false),
_dt2_check(NULL),
_dt2_bool(false)
{

}

TimeScheme::~TimeScheme(){
  delete _dt2_check;
}

void TimeScheme::reclaimTimeStep(TimeStep &timestep)
{
  // time derivative first because it is not zeroed
  _workvecs.push_back(&timestep.getTimeDerivitive());
  _workvecs.push_back(&timestep.getSolution());
}

void
TimeScheme::onTimestepBegin()
{
  if (_time_stack.empty())
  {
    _time_stack.push_back(TimeStep((_nl->_t - _dt), 0, _nl, _workvecs));
    _time_stack.back().setDt(_dt_old);
  }

  if(_dt2_bool)
  {
    //fix stack if DT2Transient
    for (int i=0; i<2; i++)
    { // reject last two short steps
      reclaimTimeStep(_time_stack.back());
      _time_stack.pop_back();
    }
    _time_stack.push_back(*_dt2_check);
    _dt2_check = NULL;
    _dt2_bool = false;
  }
  _time_stepping_scheme = _nl->_time_stepping_scheme;
  //set Solution to previous solve
  _time_stack.back().setSolution(_nl->solutionOld());
  _dt_old = _time_stack.back().getDt();
  if(_time_stack.size()!=0 && _time_stack.back().getTimeStep() == _t_step)
  {
    if(_dt != _dt_old)
    {
     //Solve Fail
      if (_dt2_check)
      {
        reclaimTimeStep(*_dt2_check);
        delete _dt2_check;
      }
      _dt2_check = new TimeStep(_time_stack.back());
      _time_stack.pop_back();
    }
    else
    {
      //DT2Transient
      _dt2_bool = true;
    }
  }
  else
  {
    //This 4 should be tied to the order of the time scheme.
    if(_time_stack.size()>4)
    {
      reclaimTimeStep(_time_stack.front());
      _time_stack.pop_front();
    }
  }
  _dt_old = _time_stack.back().getDt();
  ///Set solution to right values
  _nl->_solution.localize( _time_stack.back().getSolution());
  _nl->_solution_old.localize(_time_stack.back().getSolution());
  if(_t_step>1)
  {
    _nl->_solution_older = _time_stack[_time_stack.size()-2].getSolution();
  }
  ///push back the current time step
  _time_stack.push_back(TimeStep(_nl->_t, _t_step, _nl, _workvecs));
  _time_stack.back().setDt(_dt);
  Real sum;
  if(_t_step > 1)
  {
    _time_stack[_time_stack.size()-2].setTimeDerivitive(_solution_u_dot);
  }
  switch (_time_stepping_scheme)
  {
  case Moose::CRANK_NICOLSON:
    {
      computeLittlef(_time_stack[_time_stack.size()-2].getSolution(), _residual_old, -1, false);
      _residual_old.close();
    }
    break;

  case Moose::BDF2:
    sum = _dt+ _dt_old;
    _time_weight[0] = 1.+_dt/sum;
    _time_weight[1] =-sum/ _dt_old;
    _time_weight[2] =_dt*_dt/_dt_old/sum;
    break;

  default:
    break;
  }

}

void TimeScheme::Adams_Bashforth2P(NumericVector<Number> & initial_solution)
{
  if(!_nl->containsTimeKernel() || !_use_littlef)
  {
    if(_dt ==0 || _dt_old == 0 || _t_step<2){
      return;
    }
    initial_solution.localize(_predicted_solution);
    NumericVector<Number> & my_old_solution_u_dot = _trash1; //change to trash1
    _time_stack[_time_stack.size()-3].getTimeDerivitive().localize(my_old_solution_u_dot);
    my_old_solution_u_dot *= -1.0;
    my_old_solution_u_dot += _time_stack[_time_stack.size()-2].getTimeDerivitive();
    my_old_solution_u_dot *= (.5*_dt*_dt)/ _time_stack[_time_stack.size()-2].getDt();
    NumericVector<Number> & _old_solution_u_dot = _trash2;
    _time_stack[_time_stack.size()-2].getTimeDerivitive().localize(_old_solution_u_dot);
    _old_solution_u_dot *= _dt;
    _predicted_solution += my_old_solution_u_dot;
    _predicted_solution += _old_solution_u_dot;
    if(_apply_predictor)
    {
      my_old_solution_u_dot *= _nl->_predictor_scale;
      _old_solution_u_dot *= _nl->_predictor_scale;
      initial_solution += my_old_solution_u_dot;
      initial_solution += _old_solution_u_dot;
    }
    return;
  }
  else
  {
    NumericVector<Number> & residual_older = _trash1;
    NumericVector<Number> & residual_old = _trash2;
    initial_solution.localize(_predicted_solution);
    computeLittlef(_time_stack[_time_stack.size()-3].getSolution(), residual_older, _time_stack[_time_stack.size()-2].getTime());
    computeLittlef(_time_stack[_time_stack.size() -2].getSolution(), residual_old, _time_stack[_time_stack.size()-1].getTime());
    if(_dt ==0 || _dt_old == 0){
      std::cout<<"help me!!"<<std::endl;
    }
    residual_older *= -1.0*(_dt*_dt)/(2.0*_dt_old);
    residual_old *= -1.0*(_dt + (_dt*_dt)/(2.0*_dt_old));
    _predicted_solution +=  (residual_old);
    _predicted_solution -= residual_older;
    if(_apply_predictor)
    {
      _predicted_solution.localize(initial_solution);
    }
  }
}

Real
TimeScheme::estimateTimeError(NumericVector<Number> & solution)
{
  Real ret = -1;
  if(_dt_old >0){
    if(_use_AB2)
    {
      switch (_time_stepping_scheme)
      {
        case Moose::CRANK_NICOLSON:
        {
          _predicted_solution -= solution;
          _predicted_solution *= (_dt)/(3.0 * (_dt +_dt_old));
          ret = _predicted_solution.l2_norm();
          return ret;
        }
        case Moose::BDF2:
        {
          _predicted_solution *= -1.0;
          _predicted_solution += solution;
          Real topcalc = 2.0*(_dt + _dt_old)*(_dt +_dt_old);
          Real bottomcalc = 6.0*_dt*_dt + 12.0*_dt*_dt_old + 5.0*_dt_old*_dt_old;
          _predicted_solution *= topcalc/bottomcalc;
          ret = _predicted_solution.l2_norm();
          return ret;
        }
        case Moose::IMPLICIT_EULER:
        {
          //I am not sure this is actually correct.
          _predicted_solution *= -1;
          _predicted_solution += solution;
          Real calc = _dt*_dt*.5;
          _predicted_solution *= calc;
          ret = _predicted_solution.l2_norm();
          return ret;
        }
        default:
          break;
      }
    }
    else
    {
      switch (_time_stepping_scheme)
      {
        case Moose::IMPLICIT_EULER:
        case Moose::BDF2:
        case Moose::CRANK_NICOLSON:
        {
          //Error estimate for the FE predictor should go here.
          return ret;
        }
        default:
          break;
      }
    }
  }
  return ret;
}

void
TimeScheme::applyPredictor(NumericVector<Number> & initial_solution)
{
  // A Predictor is an algorithm that will predict the next solution based on
  // previous solutions.  Basically, it works like:
  //
  //             sol - prev_sol
  // sol = sol + -------------- * dt * scale_factor
  //                 dt_old
  //
  // The scale factor can be set to 1 for times when the solution is expected
  // to change linearly or smoothly.  If the solution is less continuous over
  // time, it may be better to set to to 0.
  //   In the ideal case of a linear model with linearly changing bcs, the Predictor
  // can determine the solution before the solver is invoked (a solution is computed
  // in zero solver iterations).  Even outside the ideal case, a good Predictor
  // significantly reduces the number of solver iterations required.
  //   It is important to compute the initial residual to be used as a relative
  // convergence criterion before applying the predictor.  If this is not done,
  // the residual is likely to be much lower after applying the predictor, which would
  // result in a much more stringent criterion for convergence than would have been
  // used if the predictor were not enabled.
  if(_use_AB2){
    if(_t_step >1)
    {
      Adams_Bashforth2P(initial_solution);
    }
    return;
  }
  if(!_use_littlef )
  {
    if (_dt_old > 0)
    {
      std::streamsize cur_precision(std::cout.precision());
      std::cout << "  Applying predictor with scale factor = "<<std::fixed<<std::setprecision(2)<<_nl->_predictor_scale<<"\n";
      std::cout << std::scientific << std::setprecision(cur_precision);
      Real dt_adjusted_scale_factor = _nl->_predictor_scale*_dt;
      NumericVector<Number> & previous_solution = _trash1;
      _time_stack[_time_stack.size()-2].getTimeDerivitive().localize(previous_solution);
      if (dt_adjusted_scale_factor != 0.0)
      {
        previous_solution *= dt_adjusted_scale_factor;
        initial_solution += previous_solution;
        initial_solution.localize(_predicted_solution);
      }
    }
  }
  else
  {
     std::streamsize cur_precision(std::cout.precision());
     std::cout << "  Applying predictor with scale factor = "<<std::fixed<<std::setprecision(2)<<_nl->_predictor_scale<<"\n";
     std::cout << std::scientific << std::setprecision(cur_precision);
     Real dt_adjusted_scale_factor = _nl->_predictor_scale *_dt;
     NumericVector<Number> & previous_solution = _trash1;
     if(dt_adjusted_scale_factor != 0.0)
     {
       computeLittlef(initial_solution, previous_solution);
       previous_solution *= dt_adjusted_scale_factor;
       initial_solution += previous_solution;
       initial_solution.localize(_predicted_solution);
     }
   }
}

void TimeScheme::computeLittlef(const NumericVector<Number> & bigF, NumericVector<Number> & littlef, Real time, bool mass)
{

  NumericVector<Number> & my_solution_u_dot = _trash3;

   if(mass)
   {
     _solution_u_dot.localize(my_solution_u_dot);
     _solution_u_dot = 1.0;
   }
  Real currenttime = _nl->_t;
  const NumericVector<Real> *current_solution = _nl->currentSolution();
  if(time != -1)
  {
    _nl->_t = time;
  }
  _nl->set_solution(bigF);// use old_solution for computing with correct solution vector
  littlef.close();

  _nl->_fe_problem.computeResidualType( bigF, littlef, Moose::KT_NONTIME);
  if(mass && _nl->containsTimeKernel())
  {
#ifdef LIBMESH_HAVE_PETSC
    _nl->computeResidualInternal( _mmatrix, Moose::KT_TIME);
    PetscVector<Number> cls((static_cast<PetscVector<Number> & > (_mmatrix)).vec());
    if( VecReciprocal(cls.vec()) != 0)
      mooseError("VecReciprocal");
    cls.close();
    littlef.pointwise_mult(littlef, _mmatrix);
    _mmatrix.close();
#else
    mooseError("Reciprocal not available");
#endif
  }
  _nl->set_solution(*current_solution);
  _nl->_t = currenttime;
  if(mass)
  {
    my_solution_u_dot.localize(_solution_u_dot);
  }
}

NumericVector<Number> & TimeScheme::finishResidual(NumericVector<Number> & residual){
  switch (_time_stepping_scheme)
  {
  case Moose::CRANK_NICOLSON:
    residual.add(_residual_old);
    residual.close();
    break;

  default:
    break;
  }
  return residual;
}

void
TimeScheme::firstOrderTD()
{
  _solution_u_dot = *_nl->currentSolution();
  _solution_u_dot -= _time_stack[_time_stack.size() -2].getSolution();
  _solution_u_dot /= _dt;

  _solution_du_dot_du = 1.0 / _dt;
}

void
TimeScheme::computeTimeDerivatives()
{
  if(_time_stack.empty())
  {
    return;
  }
  switch (_time_stepping_scheme)
  {
  case Moose::IMPLICIT_EULER:
  case Moose::EXPLICIT_EULER:
  case Moose::CRANK_NICOLSON:
    firstOrderTD();
    break;
  case Moose::BDF2:
    if (_t_step == 1)
    {
      firstOrderTD();
    }
    else
    {

      _solution_u_dot.zero();
      _solution_u_dot.add(_time_weight[0], *_nl->currentSolution());
      _solution_u_dot.add(_time_weight[1], _time_stack[_time_stack.size() -2].getSolution());

      _solution_u_dot.add(_time_weight[2], _time_stack[_time_stack.size()-3].getSolution()); //_time_stack[_time_stack.size()-2].getSolution());
      _solution_u_dot.scale(1./_dt);
      _solution_du_dot_du = _time_weight[0]/_dt;
    }
    break;

  default:
    break;
  }
  _solution_u_dot.close();
  _solution_du_dot_du.close();
}
