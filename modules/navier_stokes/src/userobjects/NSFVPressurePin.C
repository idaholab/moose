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
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_PRE_KERNELS);
  exec_enum = {EXEC_PRE_KERNELS};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  // Avoid uninitialized residual objects
  params.suppressParameter<bool>("force_preic");

  params.addParam<MooseVariableName>(NS::pressure, "Pressure variable");

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
        &UserObject::_subproblem.getVariable(0, getParam<VariableName>(NS::pressure))))
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

}
