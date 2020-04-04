//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TransientInterface.h"
#include "FEProblem.h"

InputParameters
TransientInterface::validParams()
{

  InputParameters params = emptyInputParameters();
  params.addParam<bool>(
      "implicit",
      true,
      "Determines whether this object is calculated using an implicit or explicit form");

  params.addParamNamesToGroup("implicit", "Advanced");
  return params;
}

TransientInterface::TransientInterface(const MooseObject * moose_object)
  : _ti_params(moose_object->parameters()),
    _ti_feproblem(*_ti_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _is_implicit(_ti_params.have_parameter<bool>("implicit") ? _ti_params.get<bool>("implicit")
                                                             : true),
    _t(_is_implicit ? _ti_feproblem.time() : _ti_feproblem.timeOld()),
    _t_step(_ti_feproblem.timeStep()),
    _dt(_ti_feproblem.dt()),
    _dt_old(_ti_feproblem.dtOld()),
    _is_transient(_ti_feproblem.isTransient()),
    _ti_name(MooseUtils::shortName(_ti_params.get<std::string>("_object_name")))
{
}

TransientInterface::~TransientInterface() {}
