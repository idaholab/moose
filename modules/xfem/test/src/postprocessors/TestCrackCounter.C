//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestCrackCounter.h"
#include "XFEM.h"

registerMooseObject("XFEMTestApp", TestCrackCounter);

InputParameters
TestCrackCounter::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Test postprocessor for extracting the crack_tip_origin_direction_map from XFEM.");

  return params;
}

TestCrackCounter::TestCrackCounter(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _number_of_cracks(0)
{
  FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);
  if (fe_problem == nullptr)
    mooseError("Problem casting _subproblem to FEProblemBase in TestCrackCounter");
  _xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(fe_problem->getXFEM());
  if (_xfem == nullptr)
    mooseError("Problem casting to XFEM in TestCrackCounter");
}

void
TestCrackCounter::initialize()
{
  _number_of_cracks = 0;
}

void
TestCrackCounter::execute()
{
  const std::map<const Elem *, std::vector<Point>> & crack_origins_map =
      _xfem->getCrackTipOriginMap();

  _number_of_cracks = crack_origins_map.size();
}

Real
TestCrackCounter::getValue()
{
  return _number_of_cracks;
}
