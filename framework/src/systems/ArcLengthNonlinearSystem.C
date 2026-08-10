//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArcLengthNonlinearSystem.h"

// MOOSE includes
#include "FEProblemBase.h"
#include "ResidualObject.h"

ArcLengthNonlinearSystem::ArcLengthNonlinearSystem(FEProblemBase & problem,
                                                   const std::string & name,
                                                   const TagName & load_vector_tag_name,
                                                   const TagName & load_matrix_tag_name)
  : NonlinearSystem(problem, name),
    _load_vector_tag(problem.addVectorTag(load_vector_tag_name)),
    _load_matrix_tag(problem.addMatrixTag(load_matrix_tag_name)),
    _has_load_objects(false)
{
  // GHOSTED matches the parallel type FEProblemBase gives the extra tag vectors it creates
  addVector(_load_vector_tag, false, libMesh::GHOSTED);
}

void
ArcLengthNonlinearSystem::residualAndJacobianTogether()
{
  mooseError("ArcLengthProblem does not support 'residual_and_jacobian_together'. The fused "
             "residual and Jacobian assembly path bypasses the arc-length load tag split, so the "
             "load residual would never be scaled by the load parameter lambda and the solve would "
             "silently become load control. Remove 'residual_and_jacobian_together' from the "
             "Executioner block or set it to false.");
}

void
ArcLengthNonlinearSystem::postAddResidualObject(ResidualObject & object)
{
  const auto & vtags = object.getVectorTags({});
  const auto & mtags = object.getMatrixTags({});

  if (vtags.find(_load_vector_tag) != vtags.end())
  {
    if (vtags.find(nonTimeVectorTag()) != vtags.end() || vtags.find(timeVectorTag()) != vtags.end())
      object.mooseError(
          "This object contributes to the arc-length load vector tag '",
          _fe_problem.vectorTagName(_load_vector_tag),
          "' and to a default residual vector tag at the same time, so its load would be counted "
          "twice. Mark a load object with the replacing parameter vector_tags = '",
          _fe_problem.vectorTagName(_load_vector_tag),
          "', not with extra_vector_tags.");

    _has_load_objects = true;
  }

  if (mtags.find(_load_matrix_tag) != mtags.end() && mtags.find(systemMatrixTag()) != mtags.end())
    object.mooseError(
        "This object contributes to the arc-length load matrix tag '",
        _fe_problem.matrixTagName(_load_matrix_tag),
        "' and to the default system matrix tag at the same time, so its load Jacobian would be "
        "counted twice. Mark a follower load object with the replacing parameter matrix_tags = '",
        _fe_problem.matrixTagName(_load_matrix_tag),
        "', not with extra_matrix_tags.");
}
