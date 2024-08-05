#include "SingleTraceLineSourceTest.h"
#include "NonlinearSystemBase.h"
#include "SystemBase.h"

registerMooseObject("RayTracingTestApp", SingleTraceLineSourceTest);

InputParameters
SingleTraceLineSourceTest::validParams()
{
  auto params = RepeatableRayStudy::validParams();
  params.addParam<TagName>("residual_vector_tag",
                           "the vector tag for the residual tag you will accumulate into");
  params.set<bool>("allow_other_flags_with_prekernels") = true;
  return params;
}

SingleTraceLineSourceTest::SingleTraceLineSourceTest(const InputParameters & parameters)
  : RepeatableRayStudy(parameters),
    _residual_tag_name(getParam<TagName>("residual_vector_tag")),
    _has_traced(false)
{
}

void
SingleTraceLineSourceTest::execute()
{
  if (_current_execute_flag == EXEC_TIMESTEP_BEGIN)
  {
    _has_traced = false;
    return;
  }

  if (_current_execute_flag == EXEC_NONLINEAR)
  {
    mooseAssert(_fe_problem.currentlyComputingJacobian(),
                "Should be computing jacobian but is not.");
    return;
  }

  const auto contribution_tag_id = _fe_problem.getVectorTagID(_residual_tag_name);
  auto & nl = _fe_problem.getNonlinearSystemBase(_sys.number());
  auto & contribution_vec = nl.getVector(contribution_tag_id);
  if (!_has_traced)
  {
    contribution_vec.zero();
    RayTracingStudy::execute();
    contribution_vec.close();
    _has_traced = true;
  }

  auto & residual_vec = nl.getVector(nl.residualVectorTag());
  residual_vec.close();
  residual_vec += contribution_vec;
}
