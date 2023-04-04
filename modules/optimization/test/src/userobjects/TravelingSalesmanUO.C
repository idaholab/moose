//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TravelingSalesmanUO.h"
#include "SimulatedAnnealingAlgorithm.h"

registerMooseObject("OptimizationApp", TravelingSalesmanUO);

InputParameters
TravelingSalesmanUO::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

TravelingSalesmanUO::TravelingSalesmanUO(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
}

void
TravelingSalesmanUO::execute()
{
  SimulatedAnnealingAlgorithm alg;
  alg.setObjectiveRoutine(obj, this);
  alg.solve();
}

void
TravelingSalesmanUO::obj(Real & objective,
                         const std::vector<Real> & rparams,
                         const std::vector<int> & iparams,
                         void * ctx)
{
  auto * tester = static_cast<TravelingSalesmanUO *>(ctx);
  objective = rparams[0] + (Real)iparams[0] * 0.1;
}
