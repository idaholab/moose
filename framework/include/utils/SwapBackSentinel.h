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

#ifndef SWAPBACKSENTINEL_H
#define SWAPBACKSENTINEL_H

// MOOSE includes
#include "FEProblem.h"

/**
 * The "SwapBackSentinel" class's destructor guarantees that
 * FEProblem::swapBackMaterials{Face,Neighbor}() is called even when
 * an exception is thrown from FEProblem::reinitMaterials{Face,Neighbor}.
 * This is because stack unwinding (for caught exceptions) guarantees
 * that object destructors are called.  The typical way of using this
 * object is to construct it in the same scope where reinitMaterials
 * is called:
 *
 * {
 *   SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterials, _tid);
 *   _fe_problem.reinitMaterials(_subdomain, _tid);
 * }
 */
class SwapBackSentinel
{
public:
  /**
   * SwapBackFunction is a typedef for a pointer to an FEProblem
   * member function taking a THREAD_ID and returning void.  All the
   * FEProblem::swapBackMaterialXXX() members have this signature.
   */
  using SwapBackFunction = void (FEProblem::*)(THREAD_ID);

  /**
   * Constructor taking an FEProblem reference, a function to call,
   * and the THREAD_ID argument.
   */
  SwapBackSentinel(FEProblem & fe_problem, SwapBackFunction func, THREAD_ID tid, bool predicate=true) :
      _fe_problem(fe_problem),
      _func(func),
      _thread_id(tid),
      _predicate(predicate)
  {}

  /**
   * The destructor calls swap back function only if the predicate is true.
   */
  ~SwapBackSentinel()
  {
    if (_predicate)
      (_fe_problem.*_func)(_thread_id);
  }

private:
  FEProblem & _fe_problem;
  SwapBackFunction _func;
  THREAD_ID _thread_id;
  bool _predicate;
};

#endif
