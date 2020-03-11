//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseTypes.h"

// Forward declarations
class SubProblem;

/**
 * The "SwapBackSentinel" class's destructor guarantees that
 * SubProblem::swapBackMaterials{Face,Neighbor}() is called even when
 * an exception is thrown from SubProblem::reinitMaterials{Face,Neighbor}.
 * This is because stack unwinding (for caught exceptions) guarantees
 * that object destructors are called.  The typical way of using this
 * object is to construct it in the same scope where reinitMaterials
 * is called:
 *
 * {
 *   SwapBackSentinel sentinel(_subproblem, &SubProblem::swapBackMaterials, _tid);
 *   _subproblem.reinitMaterials(_subdomain, _tid);
 * }
 */
class SwapBackSentinel
{
public:
  /**
   * SwapBackFunction is a typedef for a pointer to an SubProblem
   * member function taking a THREAD_ID and returning void.  All the
   * SubProblem::swapBackMaterialXXX() members have this signature.
   */
  using SwapBackFunction = void (SubProblem::*)(THREAD_ID);

  /**
   * Constructor taking an SubProblem reference, a function to call,
   * and the THREAD_ID argument.
   */
  SwapBackSentinel(SubProblem & subproblem,
                   SwapBackFunction func,
                   THREAD_ID tid,
                   bool predicate = true)
    : _subproblem(subproblem), _func(func), _thread_id(tid), _predicate(predicate)
  {
  }

  /**
   * The destructor calls swap back function only if the predicate is true.
   */
  ~SwapBackSentinel()
  {
    if (_predicate)
      (_subproblem.*_func)(_thread_id);
  }

private:
  SubProblem & _subproblem;
  SwapBackFunction _func;
  THREAD_ID _thread_id;
  bool _predicate;
};
