//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestADScalarKernelUserObject.h"
#include "Assembly.h"

registerMooseObject("MooseTestApp", TestADScalarKernelUserObject);

InputParameters
TestADScalarKernelUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredCoupledVar("variable", "Name of the variable that this object operates on");
  return params;
}

TestADScalarKernelUserObject::TestADScalarKernelUserObject(const InputParameters & parameters)
  : ElementUserObject(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(adCoupledValue("variable")),
    _ad_JxW(_assembly.adJxW()),
    _ad_coord(_assembly.adCoordTransformation()),
    _integral_value(0.0)
{
  _subproblem.haveADObjects(true);

  addMooseVariableDependency(&mooseVariableField());
}

void
TestADScalarKernelUserObject::initialize()
{
  _integral_value = 0.0;
}

void
TestADScalarKernelUserObject::execute()
{
  for (MooseIndex(_qrule->n_points()) qp = 0; qp < _qrule->n_points(); ++qp)
    _integral_value += _ad_JxW[qp] * _ad_coord[qp] * _u[qp];
}

void
TestADScalarKernelUserObject::threadJoin(const UserObject & y)
{
  const TestADScalarKernelUserObject & uo = static_cast<const TestADScalarKernelUserObject &>(y);
  _integral_value += uo._integral_value;
}

void
TestADScalarKernelUserObject::finalize()
{
  // Currently there is no AD gatherSum(), so this is restricted to serial execution.
  if (_communicator.size() > 1)
    mooseError("TestADScalarKernelUserObject can only be run serially.");
}

ADReal
TestADScalarKernelUserObject::getValue() const
{
  return _integral_value;
}
