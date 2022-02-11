//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADShaftTimeDerivativeScalarKernel.h"
#include "ADShaftConnectableUserObjectInterface.h"
#include "UserObject.h"

registerMooseObject("ThermalHydraulicsApp", ADShaftTimeDerivativeScalarKernel);

InputParameters
ADShaftTimeDerivativeScalarKernel::validParams()
{
  InputParameters params = ADScalarTimeDerivative::validParams();
  params.addRequiredParam<std::vector<UserObjectName>>("uo_names",
                                                       "Names of shaft-connectable user objects");

  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";

  return params;
}

ADShaftTimeDerivativeScalarKernel::ADShaftTimeDerivativeScalarKernel(
    const InputParameters & parameters)
  : ADScalarTimeDerivative(parameters),
    _uo_names(getParam<std::vector<UserObjectName>>("uo_names")),
    _n_components(_uo_names.size())
{
  _shaft_connected_uos.resize(_n_components);
  for (unsigned int i = 0; i < _n_components; ++i)
  {
    _shaft_connected_uos[i] =
        &getUserObjectByName<ADShaftConnectableUserObjectInterface>(_uo_names[i]);
  }
}

ADReal
ADShaftTimeDerivativeScalarKernel::computeQpResidual()
{
  ADReal sum_inertias = 0;
  for (unsigned int i = 0; i < _n_components; ++i)
    sum_inertias += _shaft_connected_uos[i]->getMomentOfInertia();

  return sum_inertias * _u_dot[0];
}
