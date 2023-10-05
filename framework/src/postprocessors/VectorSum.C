//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorSum.h"

// MOOSE includes
#include "NonlinearSystemBase.h"
#include "FEProblemBase.h"

// libMesh includes
#include "libmesh/numeric_vector.h"

registerMooseObject("MooseApp", VectorSum);

InputParameters
VectorSum::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<TagName>("vector", "The name of the vector to compute the sum for");
  params.addClassDescription("Computes the sum of components of the requested vector");
  return params;
}

VectorSum::VectorSum(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _vec(_fe_problem.getNonlinearSystemBase().getVector(
        _fe_problem.getVectorTagID(getParam<TagName>("vector"))))
{
}

void
VectorSum::initialize()
{
}

void
VectorSum::execute()
{
  _sum = _vec.sum();
}

PostprocessorValue
VectorSum::getValue() const
{
  return _sum;
}
