//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExceptionMaterial.h"
#include "NonlinearSystemBase.h"

registerMooseObject("MooseTestApp", ExceptionMaterial);

InputParameters
ExceptionMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("coupled_var", "Name of the coupled variable");
  params.addClassDescription("Test Material that throws MooseExceptions for testing purposes");
  params.addParam<processor_id_type>(
      "rank", DofObject::invalid_processor_id, "Isolate an exception to a particular rank");
  return params;
}

ExceptionMaterial::ExceptionMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop_value(declareProperty<Real>("matp")),
    _coupled_var(coupledValue("coupled_var")),
    _rank(getParam<processor_id_type>("rank")),
    _has_thrown(false)
{
}

void
ExceptionMaterial::computeQpProperties()
{
  // 1 + current value squared
  _prop_value[_qp] = 1.0 + _coupled_var[_qp] * _coupled_var[_qp];

  // Throw an exception if we haven't already done so, and the
  // coupled variable has reached a certain value.
  if (_fe_problem.getNonlinearSystemBase().getCurrentNonlinearIterationNumber() == 1 &&
      _t_step == 2 && !_has_thrown &&
      (_rank == DofObject::invalid_processor_id || _rank == processor_id()))
  {
    _has_thrown = true;
    throw MooseException("Exception thrown for test purposes.");
  }
}
