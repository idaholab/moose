//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReallyExpensiveFunctorMaterial.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", ReallyExpensiveFunctorMaterial);

InputParameters
ReallyExpensiveFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  return params;
}

ReallyExpensiveFunctorMaterial::ReallyExpensiveFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters)
{
  addFunctorProperty<Real>(
      "slow_prop",
      [this](const auto &, const auto &) -> Real
      {
        Real total = 0;
        for (const auto & elem : *_mesh.getActiveLocalElementRange())
          total += elem->id();
        return total / total;
      },
      std::set<ExecFlagType>(_execute_enum.begin(), _execute_enum.end()));
}
