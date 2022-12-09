//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "SolutionInvalidity.h"

/**
 * Scope guard for starting and stopping timing for a node
 *
 * Note that the SolutionGuard timing itself will take
 * approximately 0.00015 milliseconds.
 *
 * That might not sound very long - but you still don't want
 * that in the inside of tiny loops
 */
class SolutionGuard
{
public:
  /**
   * Start timing for the given ID
   *
   * @param solutioninvalidity to add warning info
   * @param id The unique id of the section
   */
  SolutionGuard(SolutionInvalidity & solutioninvalidity, const PerfID id)
    : _solutioninvalidity(solutioninvalidity)
  {
    _solutioninvalidity.push(id);
  }

  /**
   * Stop timing
   */
  ~SolutionGuard() { _solutioninvalidity.pop(); }

protected:
  ///The graph we're working on
  SolutionInvalidity & _solutioninvalidity;
};
