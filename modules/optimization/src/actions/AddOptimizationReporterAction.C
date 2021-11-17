#include "AddOptimizationReporterAction.h"

#include "Factory.h"
#include "FEProblemBase.h"
#include "OptimizationReporter.h"

registerMooseAction("isopodApp", AddOptimizationReporterAction, "add_reporter");

InputParameters
AddOptimizationReporterAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Adds OptimizationReporter objects for optimization routines.");
  return params;
}

AddOptimizationReporterAction::AddOptimizationReporterAction(InputParameters params) : MooseObjectAction(params) {}

void
AddOptimizationReporterAction::act()
{
  _problem->addReporter(_type, _name, _moose_object_pars);
}
