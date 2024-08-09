//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSPressurePin.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "NS.h"

#include "libmesh/mesh_base.h"
#include "libmesh/elem_range.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/mesh_tools.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", NSPressurePin);
registerMooseObjectRenamed("NavierStokesApp", NSFVPressurePin, "01/19/2025 00:00", NSPressurePin);

InputParameters
NSPressurePin::validParams()
{
  auto params = GeneralUserObject::validParams();
  params += BlockRestrictable::validParams();

  // Not much flexibility there, applying the pin at the wrong time prevents convergence
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  // all bad choices
  exec_enum.removeAvailableFlags(EXEC_LINEAR, EXEC_NONE, EXEC_TIMESTEP_BEGIN);
  exec_enum = {EXEC_TIMESTEP_END};

  // Avoid uninitialized residual objects
  params.suppressParameter<bool>("force_preic");

  params.addParam<NonlinearVariableName>("variable", NS::pressure, "Pressure variable");
  params.addParam<PostprocessorName>("phi0", "0", "Pressure pin value");
  MooseEnum pin_types("point-value average");
  params.addRequiredParam<MooseEnum>("pin_type", pin_types, "How to pin the pressure");
  params.addParam<Point>(
      "point",
      "The XYZ coordinates of a point inside an element where the pinned value shall be enforced.");
  params.addParam<PostprocessorName>(
      "pressure_average", "A postprocessor that computes the average of the pressure variable");

  params.addClassDescription("Pins the pressure after a solve");
  params.registerBase("Corrector");

  return params;
}

NSPressurePin::NSPressurePin(const InputParameters & params)
  : GeneralUserObject(params),
    BlockRestrictable(this),
    NonADFunctorInterface(this),
    _mesh(UserObject::_subproblem.mesh().getMesh()),
    _p(UserObject::_subproblem.getVariable(0, getParam<NonlinearVariableName>("variable"))),
    _p0(getPostprocessorValue("phi0")),
    _pressure_pin_type(getParam<MooseEnum>("pin_type")),
    _pressure_pin_point(_pressure_pin_type == "point-value" ? getParam<Point>("point")
                                                            : Point(0, 0, 0)),
    _current_pressure_average(
        _pressure_pin_type == "average" ? &getPostprocessorValue("pressure_average") : nullptr),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys"))
{
}

void
NSPressurePin::initialSetup()
{
  mooseAssert(!Threads::in_threads, "paramError is not safe in threaded mode");

  // Check execute_on of the postprocessor
  if (_pressure_pin_type == "average" &&
      !_fe_problem.getUserObjectBase(getParam<PostprocessorName>("pressure_average"))
           .getExecuteOnEnum()
           .isValueSet(getExecuteOnEnum()))
    paramError("pressure_average",
               "Pressure average postprocessor must include the pin execute_on flags");
}

void
NSPressurePin::execute()
{
  // Get the value of the pin
  Real pin_value = 0;
  if (_pressure_pin_type == "point-value")
  {
    Real point_value = _sys.system().point_value(_p.number(), _pressure_pin_point, false);

    /**
     * If we get exactly zero, we don't know if the locator couldn't find an element, or
     * if the solution is truly zero, more checking is needed.
     */
    if (MooseUtils::absoluteFuzzyEqual(point_value, 0.0))
    {
      auto pl = _mesh.sub_point_locator();
      pl->enable_out_of_mesh_mode();

      auto * elem = (*pl)(_pressure_pin_point);
      auto elem_id = elem ? elem->id() : DofObject::invalid_id;
      gatherMin(elem_id);

      if (elem_id == DofObject::invalid_id)
        mooseError("No element to gather point pressure from located at ", _pressure_pin_point);
      // Default at construction
      pl->disable_out_of_mesh_mode();
    }

    pin_value = _p0 - point_value;
  }
  else
    pin_value = _p0 - *_current_pressure_average;

  // Offset the entire pressure vector by the value of the pin
  NumericVector<Number> & sln = _sys.solution();
  std::set<dof_id_type> local_dofs;
  _sys.system().local_dof_indices(_p.number(), local_dofs);
  for (const auto dof : local_dofs)
    sln.add(dof, pin_value);
  sln.close();
  _sys.system().update();
}
