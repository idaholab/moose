//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Executioner.h"

class FlexibleExecutioner : public Executioner
{
public:
  static InputParameters validParams();

  FlexibleExecutioner(const InputParameters & parameters);

  virtual void execute() override;
  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

  virtual void preProblemInit() override
  {
    auto & ordering = getParam<std::vector<std::string>>("solve_object_ordering");
    if (ordering.size() == 0)
      mooseError("'solve_object_ordering' cannot be empty if it is given in the input");

    // Set the solve object with a name as the head solve object
    _head_solve_object = getSolveObject(ordering[0]);
    for (unsigned int i = 1; i < ordering.size(); ++i)
    {
      auto lead_object = getSolveObject(ordering[i - 1]);
      auto current_object = getSolveObject(ordering[i]);
      lead_object->setInnerSolve(*current_object);
    }

    Executioner::preProblemInit();
  }

protected:
  /**
   * Get a solve object by its name
   */
  std::shared_ptr<SolveObject> getSolveObject(const std::string & name) const
  {
    auto it = _solve_objects.find(name);
    if (it == _solve_objects.end())
      mooseError("Solve object with name ", name, " is not available.");
    return it->second;
  }

  /// The head solve object
  std::shared_ptr<SolveObject> _head_solve_object;

private:
  bool _last_solve_converged;
};
