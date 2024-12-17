//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SteadyBase.h"

// Forward declarations
class InputParameters;

/**
 * SteadySolve2 executioners call "solve()" on two different nonlinear systems in sequence
 */
class SteadySolve2 : public SteadyBase
{
public:
  static InputParameters validParams();

  SteadySolve2(const InputParameters & parameters);

  void init() override;

protected:
  class IterativeFEProblemSolve : public FEProblemSolve
  {
  public:
    IterativeFEProblemSolve(Executioner & ex,
                            const std::vector<unsigned int> & system_numbers,
                            const unsigned int niter)
      : FEProblemSolve(ex), _system_numbers(system_numbers), _number_of_iterations(niter)
    {
    }
    virtual bool solve() override
    {
      bool converged = true;
      for (unsigned int i = 0; i < _number_of_iterations; i++)
      {
        for (const auto & sys_num : _system_numbers)
        {
          _problem.solve(sys_num);

          if (_problem.shouldSolve())
          {
            if (_problem.converged(sys_num))
              _console << COLOR_GREEN << " Nonlinear system " << sys_num << " solve converged!"
                       << COLOR_DEFAULT << std::endl;
            else
            {
              _console << COLOR_RED << " Nonlinear system " << sys_num << " solve did not converge!"
                       << COLOR_DEFAULT << std::endl;
              converged = false;
            }
          }
          else
            _console << COLOR_GREEN << " Nonlinear system " << sys_num << " solve skipped!"
                     << COLOR_DEFAULT << std::endl;
        }
        if (!converged)
        {
          _console << "Aborting as solve did not converge" << std::endl;
          break;
        }
      }
      return converged;
    }

  protected:
    const std::vector<unsigned int> _system_numbers;
    const unsigned int _number_of_iterations;
  };

  const unsigned int _first_nl_sys;
  const unsigned int _second_nl_sys;
  const unsigned int _number_of_iterations;

  IterativeFEProblemSolve _feproblem_solve;
};
