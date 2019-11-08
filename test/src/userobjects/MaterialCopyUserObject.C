//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialCopyUserObject.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", MaterialCopyUserObject);

InputParameters
MaterialCopyUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<std::vector<Real>>("copy_times", "Times at which state should be copied");
  params.addRequiredParam<unsigned int>("copy_from_element",
                                        "The id of the element from which data is copied");
  params.addRequiredParam<unsigned int>("copy_to_element",
                                        "The id of the element to which data is copied");
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  return params;
}

MaterialCopyUserObject::MaterialCopyUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _copy_times(parameters.get<std::vector<Real>>("copy_times")),
    _copy_from_element(parameters.get<unsigned int>("copy_from_element")),
    _copy_to_element(parameters.get<unsigned int>("copy_to_element")),
    _time_tol(1e-8)
{
}

void
MaterialCopyUserObject::execute()
{
  for (unsigned int i = 0; i < _copy_times.size(); ++i)
  {
    if (std::abs(_t - _copy_times[i]) < _time_tol)
    {
      Elem * elem_from = _mesh.elemPtr(_copy_from_element);
      Elem * elem_to = _mesh.elemPtr(_copy_to_element);
      _material_data->copy(*elem_to, *elem_from, 0);
    }
  }
}
