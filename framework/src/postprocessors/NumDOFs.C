//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumDOFs.h"
#include "SubProblem.h"
#include "MooseStringUtils.h"

#include "libmesh/equation_systems.h"
#include "libmesh/system.h"

registerMooseObject("MooseApp", NumDOFs);

InputParameters
NumDOFs::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addParam<std::string>(
      "system",
      "ALL",
      "The system for which you want to retrieve the number of DOFs. Use ALL for all systems, "
      "NL as an alias for nl0, AUX as an alias for aux0, or the name of a specific system.");

  params.addClassDescription(
      "Return the number of Degrees of freedom from one or more equation systems.");
  return params;
}

NumDOFs::NumDOFs(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _all_systems(false),
    _system_pointer(nullptr),
    _es_pointer(nullptr)
{
  const auto system_param = getParam<std::string>("system");
  const auto system_upper = MooseUtils::toUpper(system_param);

  if (system_upper == "ALL")
  {
    _all_systems = true;
    _es_pointer = &_subproblem.es();
    return;
  }

  std::string system_name = system_param;
  if (system_upper == "NL")
    system_name = "nl0";
  else if (system_upper == "AUX")
    system_name = "aux0";

  if (!_subproblem.es().has_system(system_name))
    paramError("system", "No system found with name '", system_name, "'.");

  _system_pointer = &_subproblem.es().get_system(system_name);
}

Real
NumDOFs::getValue() const
{
  if (_all_systems)
    return _es_pointer->n_dofs();

  return _system_pointer->n_dofs();
}
