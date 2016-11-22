/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "StepperInterface.h"
#include "FEProblemBase.h"
#include "MooseTypes.h"
#include "MooseObject.h"

StepperInterface::StepperInterface(const MooseObject * moose_object) :
    DependencyResolverInterface(),
    _si_name({moose_object->parameters().get<StepperName>("_output_name") != "" ? moose_object->parameters().get<StepperName>("_output_name") : moose_object->parameters().get<std::string>("_object_name")}),
    _si_params(moose_object->parameters()),
    _si_feproblem_base(*_si_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base",
                                                                      "Missing FEProblem Pointer in StepperInterface!")),
    _default_dt(std::numeric_limits<Real>::max())
{
}

const Real &
StepperInterface::getStepperDT(const std::string & name)
{
  if (_si_params.isParamValid(name))
  {
    _depend_steppers.insert(_si_params.get<StepperName>(name));
    return _si_feproblem_base.getStepperDT(_si_params.get<StepperName>(name));
  }
  else // Send back the default
    return _default_dt;
}

const Real &
StepperInterface::getStepperDTByName(const StepperName & name)
{
  _depend_steppers.insert(name);

  return _si_feproblem_base.getStepperDT(name);
}

const std::set<std::string> &
StepperInterface::getRequestedItems()
{
  return _depend_steppers;
}

const std::set<std::string> &
StepperInterface::getSuppliedItems()
{
  return _si_name;
}

void
StepperInterface::setSuppliedItemName(const StepperName & item_name)
{
  _si_name = {static_cast<std::string>(item_name)};
}

bool
StepperInterface::hasStepper(const std::string & name) const
{
  return _si_feproblem_base.hasPostprocessor(_si_params.get<PostprocessorName>(name));
}

bool
StepperInterface::hasStepperByName(const PostprocessorName & name) const
{
  return _si_feproblem_base.hasPostprocessor(name);
}
