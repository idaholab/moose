//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVPressurePin.h"
#include "INSFVAttributes.h"
#include "GatherRCDataElementThread.h"
#include "GatherRCDataFaceThread.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "SystemBase.h"
#include "NS.h"
#include "Assembly.h"
#include "INSFVVelocityVariable.h"
#include "PiecewiseByBlockLambdaFunctor.h"
#include "VectorCompositeFunctor.h"
#include "FVElementalKernel.h"
#include "NSFVUtils.h"

#include "libmesh/mesh_base.h"
#include "libmesh/elem_range.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/remote_elem.h"
#include "metaphysicl/dualsemidynamicsparsenumberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_dynamic_std_array_wrapper.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "timpi/parallel_sync.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", NSFVPressurePin);

InputParameters
NSFVPressurePin::validParams()
{
  auto params = GeneralUserObject::validParams();
  params += TaggingInterface::validParams();
  params += BlockRestrictable::validParams();

  // Not much flexibility there, applying the pin at the wrong time does not do much
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_PRE_KERNELS);
  exec_enum = {EXEC_PRE_KERNELS};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  // Avoid uninitialized residual objects
  params.suppressParameter<bool>("force_preic");

  params.addParam<MooseVariableName>(NS::pressure, "Pressure variable");
  params.addParam<Real>("p0", "Pressure pin value");
  MooseEnum pin_types("point_value average");
  params.addRequiredParam<MooseEnum>("pin_type", pin_types, "How to pin the pressure");
  params.addParam<Point>(
      "pressure_point",
      "The XYZ coordinates of the points where the pinned value shall be enforced.");
  params.addParam<PostprocessorName>(
      "pressure_average", "A postprocessor that computes the average of the pressure variable");

  params.addClassDescription("Pins the pressure after a solve");

  return params;
}

NSFVPressurePin::NSFVPressurePin(const InputParameters & params)
  : GeneralUserObject(params),
    TaggingInterface(this),
    BlockRestrictable(this),
    NonADFunctorInterface(this),
    _moose_mesh(UserObject::_subproblem.mesh()),
    _mesh(_moose_mesh.getMesh()),
    _dim(blocksMaxDimension()),
    _p(dynamic_cast<INSFVPressureVariable *>(
        &UserObject::_subproblem.getVariable(0, getParam<VariableName>(NS::pressure)))),
    _p0(getParam<Real>("p0")),
    _pressure_pin_type(getParam<MooseEnum>("pin_type")),
    _pressure_pin_point(getParam<Point>("pressure_point")),
    _current_pressure_average(_pressure_pin_type == "average"
                                  ? &getPostprocessorValueByName("pressure_average")
                                  : nullptr)
{
  if (!_p)
    paramError(NS::pressure, "the pressure must be a INSFVPressureVariable.");
}

void
NSFVPressurePin::initialSetup()
{
}

void
NSFVPressurePin::execute()
{
  // For average pressure interpolation, we expect a checkerboard.
  // We can smooth it here by average each value with the average of several of its neighbors
  // This will change the value of the pressure pin, but if we are careful it will not change the
  // average (provided by the postprocessor)

  // Get the value of the pin
  Real pin_value = 0;
  if (_pressure_pin == "point_value")
  {
    // We query the point every time for now in case the mesh moved
    auto & pl = _mesh->point_locator();
    auto elem = pl(_pressure_pin_point, blocks());
    const auto state_arg = const auto elem_point_arg = pin_value = _p(elem_point_arg, state_arg);
  }
  else
  {
    pin_value = _p0 - *_current_pressure_average;
  }

  // Offset the entire pressure vector by the value of the pin
}
