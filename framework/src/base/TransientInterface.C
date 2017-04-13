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

#include "TransientInterface.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<TransientInterface>()
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
    _ti_feproblem(*_ti_params.get<FEProblemBase *>("_fe_problem_base")),
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
