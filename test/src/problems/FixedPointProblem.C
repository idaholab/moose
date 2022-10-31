//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FixedPointProblem.h"

#include "NonlinearSystemBase.h"

registerMooseObject("MooseTestApp", FixedPointProblem);

InputParameters
FixedPointProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addParam<TagName>("fp_tag_name", "fp_previous", "Tag name for the fixed point iteration");
  params.addParam<bool>(
      "tagged_vector_for_partial_residual",
      true,
      "Toggle between parital residual and previous solution for the tagged vector");
  return params;
}

FixedPointProblem::FixedPointProblem(const InputParameters & params)
  : FEProblem(params),
    _tagged_vector_for_partial_residual(getParam<bool>("tagged_vector_for_partial_residual")),
    _tag_previous(getParam<TagName>("fp_tag_name")),
    _tag_id(addVectorTag(_tag_previous,
                         _tagged_vector_for_partial_residual ? Moose::VECTOR_TAG_RESIDUAL
                                                             : Moose::VECTOR_TAG_SOLUTION)),
    _tagged_vector(getNonlinearSystemBase(0).addVector(_tag_id, false, GHOSTED))
{
  if (numNonlinearSystems() != 1)
    mooseError("This test problem only works with one nonlinear system");
}

void
FixedPointProblem::computeResidual(const NumericVector<Number> & soln,
                                   NumericVector<Number> & residual,
                                   const unsigned int nl_sys_num)
{
  setCurrentNonlinearSystem(nl_sys_num);

  if (_tagged_vector_for_partial_residual)
  {
    // excluding the previous tag evaluation
    getNonlinearSystemBase(nl_sys_num).disassociateVectorFromTag(_tagged_vector, _tag_id);

    const auto & residual_vector_tags = getVectorTags(Moose::VECTOR_TAG_RESIDUAL);

    _fe_vector_tags.clear();

    for (const auto & residual_vector_tag : residual_vector_tags)
      if (residual_vector_tag._id != _tag_id)
        _fe_vector_tags.insert(residual_vector_tag._id);

    computeResidualInternal(soln, residual, _fe_vector_tags);

    residual += _tagged_vector;

    getNonlinearSystemBase(nl_sys_num).associateVectorToTag(_tagged_vector, _tag_id);
  }
  else
    FEProblem::computeResidual(soln, residual);
}

void
FixedPointProblem::computeFullResidual(const NumericVector<Number> & soln,
                                       NumericVector<Number> & residual)
{
  FEProblem::computeResidual(soln, residual);
  if (_tagged_vector_for_partial_residual)
    residual += _tagged_vector;
}

void
FixedPointProblem::copySolution()
{
  // copy current solution to the tagged vector only when the tagged vector is for
  // storing the solution of previous fixed point iteration
  if (!_tagged_vector_for_partial_residual)
    _tagged_vector = getNonlinearSystemBase(0).solution();
}
