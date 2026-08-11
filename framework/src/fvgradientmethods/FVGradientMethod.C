//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVGradientMethod.h"

#include "ComputeLinearFVLimitedGradientThread.h"
#include "FEProblemBase.h"
#include "SystemBase.h"

#include "libmesh/numeric_vector.h"

namespace
{
Moose::FV::GradientLimiterType
selectGradientLimiter(const std::string & limiter_name)
{
  if (limiter_name == "none")
    return Moose::FV::GradientLimiterType::None;
  if (limiter_name == "venkatakrishnan")
    return Moose::FV::GradientLimiterType::Venkatakrishnan;

  mooseError("Linear FV gradient limiter '", limiter_name, "' is not currently supported.");
}
}

InputParameters
FVGradientMethod::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("FVGradientMethod");
  params.registerSystemAttributeName("FVGradientMethod");
  params.addClassDescription("Base class for defining cell-centered gradient methods used by "
                             "linear finite volume objects. The method produces final gradient "
                             "values, including the selected limiter when requested.");
  params.addParam<MooseEnum>("limiter",
                             MooseEnum("none venkatakrishnan", "none"),
                             "Limiter to apply to gradients produced by this method.");
  return params;
}

FVGradientMethod::FVGradientMethod(const InputParameters & params)
  : MooseObject(params), _limiter_type(selectGradientLimiter(getParam<MooseEnum>("limiter")))
{
}

void
FVGradientMethod::computeGradient(SystemBase & system,
                                  GradientContainer & gradient,
                                  const std::unordered_set<unsigned int> & variable_numbers) const
{
  for (auto & vec : gradient)
    vec->zero();

  computeGradientWithoutLimiter(system, gradient, variable_numbers);

  for (auto & vec : gradient)
    vec->close();

  if (_limiter_type == Moose::FV::GradientLimiterType::None)
    return;

  auto & fe_problem = system.feProblem();
  using ElemInfoRange = ComputeLinearFVLimitedGradientThread::ElemInfoRange;
  ElemInfoRange elem_info_range(fe_problem.mesh().ownedElemInfoBegin(),
                                fe_problem.mesh().ownedElemInfoEnd());

  PARALLEL_TRY
  {
    ComputeLinearFVLimitedGradientThread limited_gradient_thread(
        fe_problem, system, gradient, _limiter_type, variable_numbers);
    Threads::parallel_reduce(elem_info_range, limited_gradient_thread);
  }
  fe_problem.checkExceptionAndStopSolve();

  for (auto & vec : gradient)
    vec->close();
}
