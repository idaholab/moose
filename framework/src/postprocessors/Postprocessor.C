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
    _pp_moose_object(*moose_object),
    _pp_name(moose_object->name()),
    _pp_fe_problem(
        *moose_object->parameters().getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _reporter_data(_pp_fe_problem.getReporterData(ReporterData::WriteKey()))
{
  const PostprocessorReporterName r_name(_pp_name);

  // Declare the Reporter value on thread 0 only; this lets us add error checking to
  // make sure that it really is added only once
  if (moose_object->parameters().get<THREAD_ID>("_tid") == 0)
  {
    mooseAssert(!_reporter_data.hasReporterValue<PostprocessorValue>(r_name),
                "Postprocessor Reporter value is already declared");

    _reporter_data
        .declareReporterValue<PostprocessorValue, ReporterGeneralContext<PostprocessorValue>>(
            r_name, REPORTER_MODE_UNSET, *moose_object);
  }
  else
    mooseAssert(_reporter_data.hasReporterValue<PostprocessorValue>(r_name),
                "Postprocessor Reporter value is not declared");
}

PostprocessorValue
Postprocessor::getValue()
{
  return static_cast<const Postprocessor *>(this)->getValue();
}

PostprocessorValue
Postprocessor::getValue() const
{
  mooseError("getValue() (const or non-const) must be implemented.");
}

PostprocessorValue
Postprocessor::getCurrentReporterValue() const
{
  return _reporter_data.getReporterValue<PostprocessorValue>(
      PostprocessorReporterName(_pp_name), _pp_moose_object, REPORTER_MODE_ROOT);
}

typename Postprocessor::ValueType
Postprocessor::evaluate(const ElemArg & /*elem_arg*/, const Moose::StateArg & /*state*/) const
{
  return getCurrentReporterValue();
}

typename Postprocessor::ValueType
Postprocessor::evaluate(const FaceArg & /*face*/, const Moose::StateArg & /*state*/) const
{
  return getCurrentReporterValue();
}

typename Postprocessor::ValueType
Postprocessor::evaluate(const ElemQpArg & /*elem_qp*/, const Moose::StateArg & /*state*/) const
{
  return getCurrentReporterValue();
}

typename Postprocessor::ValueType
Postprocessor::evaluate(const ElemSideQpArg & /*elem_side_qp*/,
                        const Moose::StateArg & /*state*/) const
{
  return getCurrentReporterValue();
}

typename Postprocessor::ValueType
Postprocessor::evaluate(const ElemPointArg & /*elem_point_arg*/,
                        const Moose::StateArg & /*state*/) const
{
  return getCurrentReporterValue();
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
