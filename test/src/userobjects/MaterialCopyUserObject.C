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

#include "MaterialCopyUserObject.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<MaterialCopyUserObject>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredParam<std::vector<Real>>("copy_times", "Times at which state should be copied");
  params.addRequiredParam<unsigned int>("copy_from_element",
                                        "The id of the element from which data is copied");
  params.addRequiredParam<unsigned int>("copy_to_element",
                                        "The id of the element to which data is copied");

  MultiMooseEnum execute_options(MooseUtils::createExecuteOnEnum("timestep_end"));
  params.set<MultiMooseEnum>("execute_on") = execute_options;

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
