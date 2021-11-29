//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumDOFs.h"
#include "SubProblem.h"

#include "libmesh/system.h"

registerMooseObject("MooseApp", NumDOFs);

InputParameters
NumDOFs::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum system_enum("NL AUX ALL", "ALL");
  params.addParam<MooseEnum>("system",
                             system_enum,
                             "The system(s) for which you want to retrieve the number of DOFs (NL, "
                             "AUX, ALL). Default == ALL");

  params.addClassDescription(
      "Return the number of Degrees of freedom from either the NL, Aux or both systems.");
  return params;
}

NumDOFs::NumDOFs(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _system_enum(parameters.get<MooseEnum>("system").getEnum<SystemEnum>()),
    _system_pointer(nullptr),
    _es_pointer(nullptr)
{
  switch (_system_enum)
  {
    case NL:
      mooseAssert(_subproblem.es().has_system("nl0"), "No Nonlinear System found with name nl0");
      _system_pointer = &_subproblem.es().get_system("nl0");
      break;
    case AUX:
      mooseAssert(_subproblem.es().has_system("aux0"), "No Auxilary System found with name aux0");
      _system_pointer = &_subproblem.es().get_system("aux0");
      break;
    case ALL:
      _es_pointer = &_subproblem.es();
      break;
    default:
      mooseError("Unhandled enum");
  }
}

Real
NumDOFs::getValue()
{
  switch (_system_enum)
  {
    case NL:
    case AUX:
      return _system_pointer->n_dofs();
    case ALL:
      return _es_pointer->n_dofs();
    default:
      return 0;
  }
}
