//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Postprocessor.h"
#include "UserObject.h"
#include "ReporterName.h"
#include "ReporterContext.h"
#include "FEProblemBase.h"

InputParameters
Postprocessor::validParams()
{
  InputParameters params = UserObject::validParams();
  params += OutputInterface::validParams();
  params += NonADFunctorInterface::validParams();
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_TRANSFER);

  params.addParamNamesToGroup("outputs", "Advanced");
  params.registerBase("Postprocessor");
  return params;
}

Postprocessor::Postprocessor(const MooseObject * moose_object)
  : OutputInterface(moose_object->parameters()),
    NonADFunctorInterface(moose_object),
    Moose::FunctorBase<Real>(moose_object->name()),
    _pp_name(moose_object->name()),
    _current_value(declareValue(*moose_object)),
    _pp_moose_object(*moose_object)
{
}

const PostprocessorValue &
Postprocessor::declareValue(const MooseObject & moose_object)
{
  auto & fe_problem =
      *moose_object.parameters().getCheckedPointerParam<FEProblemBase *>("_fe_problem_base");

  const PostprocessorReporterName r_name(_pp_name);

  const bool is_thread_0 = moose_object.parameters().get<THREAD_ID>("_tid") == 0;
  mooseAssert(is_thread_0 ==
                  !fe_problem.getReporterData().hasReporterValue<PostprocessorValue>(r_name),
              "Postprocessor Reporter threaded value declaration mismatch");

  // Declare the Reporter value on thread 0 only; this lets us add error checking to
  // make sure that it really is added only once
  if (is_thread_0)
    fe_problem.getReporterData(ReporterData::WriteKey())
        .declareReporterValue<PostprocessorValue, ReporterGeneralContext<PostprocessorValue>>(
            r_name, REPORTER_MODE_UNSET, moose_object);

  // At this point, thread 0 should have declared the value and getting it should be valid
  return fe_problem.getReporterData().getReporterValue<PostprocessorValue>(r_name);
}

PostprocessorValue
Postprocessor::getValue()
{
  mooseError(
      "The non-const 'Postprocessor::getValue()' method is deprecated and should not be called.");
}

PostprocessorValue
Postprocessor::getValue() const
{
  mooseDeprecated("The non-const 'Postprocessor::getValue()' method is deprecated. Please override "
                  "the 'Postprocessor::getValue() const' method instead.");

  return const_cast<Postprocessor *>(this)->getValue();
}

typename Postprocessor::ValueType
Postprocessor::evaluate(const ElemArg & /*elem_arg*/, const Moose::StateArg & /*state*/) const
{
  return getCurrentValue();
}

typename Postprocessor::ValueType
Postprocessor::evaluate(const FaceArg & /*face*/, const Moose::StateArg & /*state*/) const
{
  return getCurrentValue();
}

typename Postprocessor::ValueType
Postprocessor::evaluate(const ElemQpArg & /*elem_qp*/, const Moose::StateArg & /*state*/) const
{
  return getCurrentValue();
}

typename Postprocessor::ValueType
Postprocessor::evaluate(const ElemSideQpArg & /*elem_side_qp*/,
                        const Moose::StateArg & /*state*/) const
{
  return getCurrentValue();
}

typename Postprocessor::ValueType
Postprocessor::evaluate(const ElemPointArg & /*elem_point_arg*/,
                        const Moose::StateArg & /*state*/) const
{
  return getCurrentValue();
}

typename Postprocessor::GradientType
Postprocessor::evaluateGradient(const ElemArg & /*elem_arg*/,
                                const Moose::StateArg & /*state*/) const
{
  return 0;
}

typename Postprocessor::GradientType
Postprocessor::evaluateGradient(const FaceArg & /*face*/, const Moose::StateArg & /*state*/) const
{
  return 0;
}

typename Postprocessor::GradientType
Postprocessor::evaluateGradient(const ElemQpArg & /*elem_qp*/,
                                const Moose::StateArg & /*state*/) const
{
  return 0;
}

typename Postprocessor::GradientType
Postprocessor::evaluateGradient(const ElemSideQpArg & /*elem_side_qp*/,
                                const Moose::StateArg & /*state*/) const
{
  return 0;
}

typename Postprocessor::GradientType
Postprocessor::evaluateGradient(const ElemPointArg & /*elem_point_arg*/,
                                const Moose::StateArg & /*state*/) const
{
  return 0;
}

typename Postprocessor::DotType
Postprocessor::evaluateDot(const ElemArg & /*elem_arg*/, const Moose::StateArg & /*state*/) const
{
  evaluateDotWarning();
  return 0;
}

typename Postprocessor::DotType
Postprocessor::evaluateDot(const FaceArg & /*face*/, const Moose::StateArg & /*state*/) const
{
  evaluateDotWarning();
  return 0;
}

typename Postprocessor::DotType
Postprocessor::evaluateDot(const ElemQpArg & /*elem_qp*/, const Moose::StateArg & /*state*/) const
{
  evaluateDotWarning();
  return 0;
}

typename Postprocessor::DotType
Postprocessor::evaluateDot(const ElemSideQpArg & /*elem_side_qp*/,
                           const Moose::StateArg & /*state*/) const
{
  evaluateDotWarning();
  return 0;
}

typename Postprocessor::DotType
Postprocessor::evaluateDot(const ElemPointArg & /*elem_point_arg*/,
                           const Moose::StateArg & /*state*/) const
{
  evaluateDotWarning();
  return 0;
}

void
Postprocessor::evaluateDotWarning() const
{
  mooseDoOnce(_pp_moose_object.mooseWarning(
      "The time derivative functor operator was called on this post-processor.\n\nA zero value "
      "will always be returned, even if the post-processor value changes with time."));
}
