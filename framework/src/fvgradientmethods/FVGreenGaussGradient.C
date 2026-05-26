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

void
FVGreenGaussGradient::computeGradientWithoutLimiter(
    SystemBase & system,
    GradientContainer & output_gradient,
    GradientContainer &,
    const std::unordered_set<unsigned int> & variable_numbers) const
{
  auto & fe_problem = system.feProblem();

  PARALLEL_TRY
  {
    using FaceInfoRange = ComputeLinearFVGreenGaussGradientFaceThread::FaceInfoRange;
    FaceInfoRange face_info_range(fe_problem.mesh().ownedFaceInfoBegin(),
                                  fe_problem.mesh().ownedFaceInfoEnd());

    ComputeLinearFVGreenGaussGradientFaceThread gradient_face_thread(
        fe_problem, system, output_gradient, variable_numbers, true);
    Threads::parallel_reduce(face_info_range, gradient_face_thread);
  }
  fe_problem.checkExceptionAndStopSolve();

  for (auto & vec : output_gradient)
    vec->close();

  PARALLEL_TRY
  {
    using ElemInfoRange = ComputeLinearFVGreenGaussGradientVolumeThread::ElemInfoRange;
    ElemInfoRange elem_info_range(fe_problem.mesh().ownedElemInfoBegin(),
                                  fe_problem.mesh().ownedElemInfoEnd());

    ComputeLinearFVGreenGaussGradientVolumeThread gradient_volume_thread(
        fe_problem, system, output_gradient, variable_numbers, true);
    Threads::parallel_reduce(elem_info_range, gradient_volume_thread);
  }
  fe_problem.checkExceptionAndStopSolve();
}
