//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestSerializedSolution.h"

#include "SystemBase.h"

#include "libmesh/numeric_vector.h"

registerMooseObject("MooseTestApp", TestSerializedSolution);

InputParameters
TestSerializedSolution::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  MooseEnum system("nl aux");

  params.addParam<MooseEnum>("system", system, "Which system to test");

  return params;
}

TestSerializedSolution::TestSerializedSolution(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _test_sys(getParam<MooseEnum>("system") == 0
                  ? (SystemBase &)_fe_problem.getNonlinearSystemBase()
                  : (SystemBase &)_fe_problem.getAuxiliarySystem()),
    _serialized_solution(_test_sys.serializedSolution()),
    _sum(0)
{
}

void
TestSerializedSolution::initialize()
{
  _sum = 0;
}

void
TestSerializedSolution::execute()
{
  if (_serialized_solution.size() != _test_sys.system().n_dofs())
    mooseError("Serialized solution vector doesn't contain the correct number of entries!");

  // Sum up all entries in the solution vector
  for (unsigned int i = 0; i < _serialized_solution.size(); i++)
    _sum += _serialized_solution(i);

  // Verify that every processor got the same value
  // Note: the arguments to this "if" MUST be in this order!
  if (!_communicator.verify(_sum) && processor_id() == 0)
    mooseError("Serialized solution vectors are not the same on all processors!");
}

Real
TestSerializedSolution::getValue()
{
  return _sum;
}
