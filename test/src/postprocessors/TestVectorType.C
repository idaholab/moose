//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestVectorType.h"

#include "SystemBase.h"

#include "libmesh/numeric_vector.h"

using namespace libMesh;

registerMooseObject("MooseTestApp", TestVectorType);

InputParameters
TestVectorType::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  MooseEnum system("nl aux");

  params.addParam<MooseEnum>("system", system, "Which system to test");

  params.addParam<std::string>("vector", "Which vector to test");

  MooseEnum vec_type("automatic serial parallel ghosted"); // matches libMesh order

  params.addParam<MooseEnum>("vector_type", vec_type, "Which vector type to expect");

  return params;
}

TestVectorType::TestVectorType(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _test_sys(getParam<MooseEnum>("system") == 0
                  ? (SystemBase &)_fe_problem.getNonlinearSystemBase(/*nl_sys_num=*/0)
                  : (SystemBase &)_fe_problem.getAuxiliarySystem()),
    _test_vec_name(getParam<std::string>("vector")),
    _par_type(getParam<MooseEnum>("vector_type").getEnum<ParallelType>())
{
}

void
TestVectorType::execute()
{
  const NumericVector<Number> & test_vec = _test_sys.system().get_vector(_test_vec_name);

  if (test_vec.type() == GHOSTED && _par_type != GHOSTED)
    mooseError("Specified non-ghosted vector has been ghosted!");

  if (test_vec.type() == SERIAL && _par_type == GHOSTED && test_vec.n_processors() > 1)
    mooseError("Specified ghosted vector has been serialized!");

  if (test_vec.type() == SERIAL && _par_type == PARALLEL && test_vec.n_processors() > 1)
    mooseError("Specified parallel vector has been serialized!");
}

Real
TestVectorType::getValue() const
{
  return 1;
}
