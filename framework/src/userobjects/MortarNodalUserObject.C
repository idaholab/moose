//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarNodalUserObject.h"
#include "MooseVariableField.h"
#include "MortarUtils.h"
#include "MooseUtils.h"
#include "libmesh/quadrature.h"

namespace
{
const InputParameters &
setBoundaryParam(const InputParameters & params_in)
{
  InputParameters & ret = const_cast<InputParameters &>(params_in);
  ret.set<std::vector<BoundaryName>>("boundary") = {
      params_in.get<BoundaryName>("secondary_boundary")};
  return ret;
}
}

InputParameters
MortarNodalUserObject::validParams()
{
  InputParameters params = NodalUserObject::validParams();
  params += MortarConsumerInterface::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params.set<bool>("ghost_point_neighbors") = true;
  params.suppressParameter<std::vector<BoundaryName>>("boundary");
  params.suppressParameter<std::vector<SubdomainName>>("block");

  // We should probably default use_displaced_mesh to true. If no displaced mesh exists
  // FEProblemBase::addKernel will automatically correct it to false. However,
  // this will still prompt a call from AugmentSparsityOnInterface to get a displaced
  // mortar interface since object._use_displaced_mesh = true.

  return params;
}

MortarNodalUserObject::MortarNodalUserObject(const InputParameters & parameters)
  : NodalUserObject(setBoundaryParam(parameters)),
    MortarConsumerInterface(this),
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, getBoundaryIDs()),
    _displaced(getParam<bool>("use_displaced_mesh")),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
}

void
MortarNodalUserObject::initialSetup()
{
  std::array<const MortarNodalUserObject *, 1> consumers = {{this}};

  Moose::Mortar::setupMortarMaterials(consumers,
                                      _fe_problem,
                                      amg(),
                                      _tid,
                                      _secondary_ip_sub_to_mats,
                                      _primary_ip_sub_to_mats,
                                      _secondary_boundary_mats);
}

void
MortarNodalUserObject::execute()
{
  const auto & its = amg().secondariesToMortarSegments(*_current_node);

  auto act_functor = [this]() { executeMortarSegment(); };

  std::array<MortarNodalUserObject *, 1> consumers = {{this}};

  Moose::Mortar::loopOverMortarSegments(its,
                                        _assembly,
                                        _subproblem,
                                        _fe_problem,
                                        amg(),
                                        _displaced,
                                        consumers,
                                        _tid,
                                        _secondary_ip_sub_to_mats,
                                        _primary_ip_sub_to_mats,
                                        _secondary_boundary_mats,
                                        act_functor);
}
