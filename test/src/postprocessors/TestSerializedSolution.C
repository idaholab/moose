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

#include "TestSerializedSolution.h"

#include "SystemBase.h"

#include "libmesh/numeric_vector.h"

template <>
InputParameters
validParams<TestSerializedSolution>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

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
