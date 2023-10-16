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
  auto params = GeneralUserObject::validParams();
  params += TaggingInterface::validParams();
  params += BlockRestrictable::validParams();
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
  : GeneralUserObject(params),
    TaggingInterface(this),
    BlockRestrictable(this),
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
    _sys(*getCheckedPointerParam<SystemBase *>("_sys"))
{
  if (!_p)
    paramError(NS::pressure, "the pressure must be a INSFVPressureVariable.");

  auto check_blocks = [this](const auto & var)
  {
    const auto & var_blocks = var.blockIDs();
    const auto & uo_blocks = blockIDs();

    // Error if this UO has any blocks that the variable does not
    std::set<SubdomainID> uo_blocks_minus_var_blocks;
    std::set_difference(
        uo_blocks.begin(),
        uo_blocks.end(),
        var_blocks.begin(),
        var_blocks.end(),
        std::inserter(uo_blocks_minus_var_blocks, uo_blocks_minus_var_blocks.end()));
    if (uo_blocks_minus_var_blocks.size() > 0)
      mooseError("Block restriction of interpolator user object '",
                 this->name(),
                 "' (",
                 Moose::stringify(blocks()),
                 ") includes blocks not in the block restriction of variable '",
                 var.name(),
                 "' (",
                 Moose::stringify(var.blocks()),
                 ")");

    // Get the blocks in the variable but not this UO
    std::set<SubdomainID> var_blocks_minus_uo_blocks;
    std::set_difference(
        var_blocks.begin(),
        var_blocks.end(),
        uo_blocks.begin(),
        uo_blocks.end(),
        std::inserter(var_blocks_minus_uo_blocks, var_blocks_minus_uo_blocks.end()));

    // For each block in the variable but not this UO, error if there is connection
    // to any blocks on the UO.
    for (auto & block_id : var_blocks_minus_uo_blocks)
    {
      const auto connected_blocks = _moose_mesh.getBlockConnectedBlocks(block_id);
      std::set<SubdomainID> connected_blocks_on_uo;
      std::set_intersection(connected_blocks.begin(),
                            connected_blocks.end(),
                            uo_blocks.begin(),
                            uo_blocks.end(),
                            std::inserter(connected_blocks_on_uo, connected_blocks_on_uo.end()));
      if (connected_blocks_on_uo.size() > 0)
        mooseError("Block restriction of interpolator user object '",
                   this->name(),
                   "' (",
                   Moose::stringify(uo_blocks),
                   ") doesn't match the block restriction of variable '",
                   var.name(),
                   "' (",
                   Moose::stringify(var_blocks),
                   ")");
    }
  };
  check_blocks(*_p);

  if (!_u)
    paramError("u", "the u velocity must be an INSFVVelocityVariable.");
  check_blocks(*_u);
  _var_numbers.push_back(_u->number());

  if (_dim >= 2)
  {
    if (!_v)
      mooseError("In two or more dimensions, the v velocity must be supplied and it must be an "
                 "INSFVVelocityVariable.");
    check_blocks(*_v);
    _var_numbers.push_back(_v->number());
  }

  if (_dim >= 3)
  {
    if (!_w)
      mooseError("In three-dimensions, the w velocity must be supplied and it must be an "
                 "INSFVVelocityVariable.");
    check_blocks(*_w);
    _var_numbers.push_back(_w->number());
  }

  auto fill_container = [this](const auto & name, auto & container)
  {
    for (const auto tid : make_range(libMesh::n_threads()))
    {
      auto * const var = static_cast<MooseVariableFVReal *>(
          &UserObject::_subproblem.getVariable(tid, getParam<VariableName>(name)));
      container[tid] = var;
    }
  };

  fill_container(NS::pressure, _ps);
  fill_container("u", _us);

  if (_dim >= 2)
  {
    fill_container("v", _vs);
    if (_v->faceInterpolationMethod() != _u->faceInterpolationMethod())
      mooseError("x and y velocity component face interpolation methods do not match");

    if (_dim >= 3)
    {
      fill_container("w", _ws);
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

bool
RhieChowInterpolatorBase::hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const
{
  if (fi_elem_side)
    return hasBlocks(fi.elem().subdomain_id());
  else
    return fi.neighborPtr() && hasBlocks(fi.neighbor().subdomain_id());
}
