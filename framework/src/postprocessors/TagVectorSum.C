//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagVectorSum.h"

// MOOSE includes
#include "NonlinearSystemBase.h"
#include "FEProblemBase.h"

// libMesh includes
#include "libmesh/numeric_vector.h"

registerMooseObject("MooseApp", TagVectorSum);

InputParameters
TagVectorSum::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<TagName>("vector", "The name of the vector to compute the sum for");
  params.addClassDescription("Computes the sum of components of the requested tagged vector");
  return params;
}

TagVectorSum::TagVectorSum(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _vec(_fe_problem.getNonlinearSystemBase(_sys.number())
             .getVector(_fe_problem.getVectorTagID(getParam<TagName>("vector"))))
{
}

void
TagVectorSum::initialize()
{
}

void
TagVectorSum::execute()
{
  _sum = _vec.sum();
}

PostprocessorValue
TagVectorSum::getValue() const
{
  return _sum;
}
