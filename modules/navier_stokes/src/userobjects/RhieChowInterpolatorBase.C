//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RhieChowInterpolatorBase.h"
#include "INSFVAttributes.h"
#include "GatherRCDataElementThread.h"
#include "GatherRCDataFaceThread.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "SystemBase.h"
#include "NS.h"
#include "Assembly.h"
#include "INSFVVelocityVariable.h"
#include "INSFVPressureVariable.h"
#include "PiecewiseByBlockLambdaFunctor.h"
#include "VectorCompositeFunctor.h"
#include "FVElementalKernel.h"

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

InputParameters
RhieChowInterpolatorBase::validParams()
{
  auto params = RhieChowFaceFluxProvider::validParams();
  params += TaggingInterface::validParams();
  params += ADFunctorInterface::validParams();

  params.addClassDescription(
      "Computes the Rhie-Chow velocity based on gathered 'a' coefficient data.");

  // Avoid uninitialized residual objects
  params.suppressParameter<bool>("force_preic");

  params.addRequiredParam<VariableName>(NS::pressure, "The pressure variable.");
  params.addRequiredParam<VariableName>("u", "The x-component of velocity");
  params.addParam<VariableName>("v", "The y-component of velocity");
  params.addParam<VariableName>("w", "The z-component of velocity");

  MooseEnum velocity_interp_method("average rc", "rc");
  params.addParam<MooseEnum>(
      "velocity_interp_method",
      velocity_interp_method,
      "The interpolation to use for the velocity. Options are "
      "'average' and 'rc' which stands for Rhie-Chow. The default is Rhie-Chow.");

  return params;
}

RhieChowInterpolatorBase::RhieChowInterpolatorBase(const InputParameters & params)
  : RhieChowFaceFluxProvider(params),
    TaggingInterface(this),
    ADFunctorInterface(this),
    _moose_mesh(UserObject::_subproblem.mesh()),
    _mesh(_moose_mesh.getMesh()),
    _dim(blocksMaxDimension()),
    _p(dynamic_cast<INSFVPressureVariable *>(
        &UserObject::_subproblem.getVariable(0, getParam<VariableName>(NS::pressure)))),
    _u(dynamic_cast<INSFVVelocityVariable *>(
        &UserObject::_subproblem.getVariable(0, getParam<VariableName>("u")))),
    _v(isParamValid("v") ? dynamic_cast<INSFVVelocityVariable *>(
                               &UserObject::_subproblem.getVariable(0, getParam<VariableName>("v")))
                         : nullptr),
    _w(isParamValid("w") ? dynamic_cast<INSFVVelocityVariable *>(
                               &UserObject::_subproblem.getVariable(0, getParam<VariableName>("w")))
                         : nullptr),
    _ps(libMesh::n_threads(), nullptr),
    _us(libMesh::n_threads(), nullptr),
    _vs(libMesh::n_threads(), nullptr),
    _ws(libMesh::n_threads(), nullptr),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _displaced(dynamic_cast<DisplacedProblem *>(&(UserObject::_subproblem)))
{
  if (!_p)
    paramError(NS::pressure, "the pressure must be a INSFVPressureVariable.");
  checkBlocks(*_p);

  if (!_u)
    paramError("u", "the u velocity must be an INSFVVelocityVariable.");
  checkBlocks(*_u);
  _var_numbers.push_back(_u->number());

  if (_dim >= 2)
  {
    if (!_v)
      mooseError("In two or more dimensions, the v velocity must be supplied and it must be an "
                 "INSFVVelocityVariable.");
    checkBlocks(*_v);
    _var_numbers.push_back(_v->number());
  }

  if (_dim >= 3)
  {
    if (!_w)
      mooseError("In three-dimensions, the w velocity must be supplied and it must be an "
                 "INSFVVelocityVariable.");
    checkBlocks(*_w);
    _var_numbers.push_back(_w->number());
  }

  fillContainer(NS::pressure, _ps);
  fillContainer("u", _us);

  if (_dim >= 2)
  {
    fillContainer("v", _vs);
    if (_v->faceInterpolationMethod() != _u->faceInterpolationMethod())
      mooseError("x and y velocity component face interpolation methods do not match");

    if (_dim >= 3)
    {
      fillContainer("w", _ws);
      if (_w->faceInterpolationMethod() != _u->faceInterpolationMethod())
        mooseError("x and z velocity component face interpolation methods do not match");
    }
  }

  if (&(UserObject::_subproblem) != &(TaggingInterface::_subproblem))
    mooseError("Different subproblems in RhieChowInterpolatorBase!");

  const auto & velocity_interp_method = params.get<MooseEnum>("velocity_interp_method");
  if (velocity_interp_method == "average")
    _velocity_interp_method = Moose::FV::InterpMethod::Average;
  else if (velocity_interp_method == "rc")
    _velocity_interp_method = Moose::FV::InterpMethod::RhieChow;
}

Real
RhieChowInterpolatorBase::getVolumetricFaceFlux(const Moose::FV::InterpMethod m,
                                                const FaceInfo & fi,
                                                const Moose::StateArg & time,
                                                const THREAD_ID tid,
                                                bool subtract_mesh_velocity) const
{
  return raw_value(this->getVelocity(m, fi, time, tid, subtract_mesh_velocity)) * fi.normal();
}
