//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVGreenGaussGradient.h"

#include "ComputeLinearFVGreenGaussGradientFaceThread.h"
#include "ComputeLinearFVGreenGaussGradientVolumeThread.h"
#include "FEProblemBase.h"
#include "MathFVUtils.h"
#include "SystemBase.h"

registerMooseObject("MooseApp", FVGreenGaussGradient);

InputParameters
FVGreenGaussGradient::validParams()
{
  InputParameters params = FVGradientMethod::validParams();
  params.addClassDescription("Green-Gauss cell-centered gradient method.");
  return params;
}

FVGreenGaussGradient::FVGreenGaussGradient(const InputParameters & params)
  : FVGradientMethod(params)
{
}

FVGreenGaussGradient::InternalFaceValues
FVGreenGaussGradient::internalFaceValues(const FaceInfo & fi,
                                         const ElemInfo & /*elem_info*/,
                                         const ElemInfo & /*neighbor_info*/,
                                         Real elem_value,
                                         Real neighbor_value) const
{
  const Real face_value = Moose::FV::linearInterpolation(elem_value, neighbor_value, fi, true);
  return {face_value, face_value};
}

void
FVGreenGaussGradient::computeGradientWithoutLimiter(
    SystemBase & system,
    GradientContainer & gradient,
    const std::unordered_set<unsigned int> & variable_numbers) const
{
  auto & fe_problem = system.feProblem();

  PARALLEL_TRY
  {
    using FaceInfoRange = ComputeLinearFVGreenGaussGradientFaceThread::FaceInfoRange;
    FaceInfoRange face_info_range(fe_problem.mesh().ownedFaceInfoBegin(),
                                  fe_problem.mesh().ownedFaceInfoEnd());

    ComputeLinearFVGreenGaussGradientFaceThread gradient_face_thread(
        fe_problem, system, gradient, variable_numbers, *this);
    Threads::parallel_reduce(face_info_range, gradient_face_thread);
  }
  fe_problem.checkExceptionAndStopSolve();

  for (auto & vec : gradient)
    vec->close();

  PARALLEL_TRY
  {
    using ElemInfoRange = ComputeLinearFVGreenGaussGradientVolumeThread::ElemInfoRange;
    ElemInfoRange elem_info_range(fe_problem.mesh().ownedElemInfoBegin(),
                                  fe_problem.mesh().ownedElemInfoEnd());

    ComputeLinearFVGreenGaussGradientVolumeThread gradient_volume_thread(
        fe_problem, system, gradient, variable_numbers);
    Threads::parallel_reduce(elem_info_range, gradient_volume_thread);
  }
  fe_problem.checkExceptionAndStopSolve();
}
