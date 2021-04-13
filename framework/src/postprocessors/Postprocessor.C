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

defineLegacyParams(Postprocessor);

InputParameters
Postprocessor::validParams()
{
  InputParameters params = UserObject::validParams();
  params += OutputInterface::validParams();

  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_TRANSFER);

  params.addParamNamesToGroup("outputs", "Advanced");
  params.registerBase("Postprocessor");
  return params;
}

Postprocessor::Postprocessor(const MooseObject * moose_object)
  : OutputInterface(moose_object->parameters()), _pp_name(moose_object->name())
{
  auto & fe_problem =
      *moose_object->parameters().getCheckedPointerParam<FEProblemBase *>("_fe_problem_base");

  const PostprocessorReporterName r_name(moose_object->name());

  mooseAssert(!fe_problem.getReporterData().hasReporterValue<PostprocessorValue>(r_name),
              "Postprocessor Reporter value is already declared");

  // Declare the Reporter value on thread 0 only; this lets us add error checking to
  // make sure that it really is added only once
  const auto tid = moose_object->parameters().isParamValid("_tid")
                       ? moose_object->parameters().get<THREAD_ID>("_tid")
                       : 0;
  if (tid == 0)
    fe_problem.getReporterData(ReporterData::WriteKey())
        .declareReporterValue<PostprocessorValue, ReporterGeneralContext<PostprocessorValue>>(
            r_name, REPORTER_MODE_UNSET, *moose_object);
}
