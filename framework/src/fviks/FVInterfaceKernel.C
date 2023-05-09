//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInterfaceKernel.h"
#include "FEProblemBase.h"
#include "SystemBase.h"
#include "MooseVariableFV.h"
#include "Assembly.h"

InputParameters
FVInterfaceKernel::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += BoundaryRestrictableRequired::validParams();
  params += SetupInterface::validParams();
  params += FunctionInterface::validParams();
  params += DistributionInterface::validParams();
  params += UserObjectInterface::validParams();
  params += TransientInterface::validParams();
  params += PostprocessorInterface::validParams();
  params += VectorPostprocessorInterface::validParams();
  params += GeometricSearchInterface::validParams();
  params += MeshChangedInterface::validParams();
  params += TaggingInterface::validParams();
  params += NeighborCoupleableMooseVariableDependencyIntermediateInterface::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params += ADFunctorInterface::validParams();

  params.addRequiredParam<std::vector<SubdomainName>>(
      "subdomain1", "The subdomains on the 1st side of the boundary.");
  params.addRequiredParam<std::vector<SubdomainName>>(
      "subdomain2", "The subdomains on the 2nd side of the boundary.");
  params.addRequiredParam<NonlinearVariableName>(
      "variable1", "The name of the first variable that this interface kernel applies to");
  params.addParam<NonlinearVariableName>(
      "variable2",
      "The name of the second variable that this interface kernel applies to. If not supplied, "
      "variable1 will be used.");
  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");

  params.addParam<unsigned short>("ghost_layers", 1, "The number of layers of elements to ghost.");
  params.addParam<bool>("use_point_neighbors",
                        false,
                        "Whether to use point neighbors, which introduces additional ghosting to "
                        "that used for simple face neighbors.");
  params.addParamNamesToGroup("ghost_layers use_point_neighbors", "Parallel ghosting");

  // FV Interface Kernels always need one layer of ghosting because the elements
  // on each side of the interface may be on different MPI ranks, but we still
  // need to access them as a pair to compute the numerical face flux.
  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
          Moose::RelationshipManagerType::COUPLING,
      [](const InputParameters & obj_params, InputParameters & rm_params)
      {
        rm_params.set<unsigned short>("layers") = obj_params.get<unsigned short>("ghost_layers");
        rm_params.set<bool>("use_point_neighbors") = obj_params.get<bool>("use_point_neighbors");
      });

  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  params.addCoupledVar("displacements", "The displacements");
  params.declareControllable("enable");
  params.registerBase("FVInterfaceKernel");
  params.registerSystemAttributeName("FVInterfaceKernel");
  params.set<bool>("_residual_object") = true;
  return params;
}

FVInterfaceKernel::FVInterfaceKernel(const InputParameters & parameters)
  : MooseObject(parameters),
    BoundaryRestrictableRequired(this, false),
    SetupInterface(this),
    FunctionInterface(this),
    DistributionInterface(this),
    UserObjectInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    VectorPostprocessorInterface(this),
    GeometricSearchInterface(this),
    MeshChangedInterface(parameters),
    TaggingInterface(this),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(
        this, /*nodal=*/false, /*neighbor_nodal=*/false, /*is_fv=*/true),
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
    ADFunctorInterface(this),
    _tid(getParam<THREAD_ID>("_tid")),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _assembly(_subproblem.assembly(_tid, _sys.number())),
    _var1(_sys.getFVVariable<Real>(_tid, getParam<NonlinearVariableName>("variable1"))),
    _var2(_sys.getFVVariable<Real>(_tid,
                                   isParamValid("variable2")
                                       ? getParam<NonlinearVariableName>("variable2")
                                       : getParam<NonlinearVariableName>("variable1"))),
    _mesh(_subproblem.mesh())
{
  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh", "FV interface kernels do not yet support displaced mesh");

  _subproblem.haveADObjects(true);
  addMooseVariableDependency(&_var1);
  addMooseVariableDependency(&_var2);

  for (const auto & sub_name : getParam<std::vector<SubdomainName>>("subdomain1"))
    _subdomain1.insert(_mesh.getSubdomainID(sub_name));
  for (const auto & sub_name : getParam<std::vector<SubdomainName>>("subdomain2"))
    _subdomain2.insert(_mesh.getSubdomainID(sub_name));

  if (!_var1.hasBlocks(_subdomain1))
    paramError("variable1",
               "variable1 does not exist on all the blocks specified by the subdomain1 parameter");
  if (!_var2.hasBlocks(_subdomain2))
  {
    const bool var2_provided = isParamValid("variable2");
    const std::string var_name = var2_provided ? "variable2" : "variable1";
    paramError(var_name,
               var_name,
               " does not exist on all the blocks specified by the subdomain2 parameter",
               var2_provided ? ""
                             : ".\nNote that you did not provide the variable2 parameter, "
                               "so variable1 was implicitly used on subdomain2");
  }
}

void
FVInterfaceKernel::setupData(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  _elem_is_one = _subdomain1.find(fi.elem().subdomain_id()) != _subdomain1.end();

#ifndef NDEBUG
  const auto ft1 = fi.faceType(_var1.name());
  const auto ft2 = fi.faceType(_var2.name());
  constexpr auto ft_both = FaceInfo::VarFaceNeighbors::BOTH;
  constexpr auto ft_elem = FaceInfo::VarFaceNeighbors::ELEM;
  constexpr auto ft_neigh = FaceInfo::VarFaceNeighbors::NEIGHBOR;
  mooseAssert(
      (_elem_is_one && (ft1 == ft_elem || ft1 == ft_both) && (ft2 == ft_neigh || ft2 == ft_both)) ||
          (!_elem_is_one && (ft1 == ft_neigh || ft1 == ft_both) &&
           (ft2 == ft_elem || ft2 == ft_both)),
      "Face type was not recognized. Check that the specified boundaries are interfaces.");
#endif
}

void
FVInterfaceKernel::processResidual(const Real resid,
                                   const unsigned int var_num,
                                   const bool neighbor)
{
  neighbor ? prepareVectorTagNeighbor(_assembly, var_num) : prepareVectorTag(_assembly, var_num);
  _local_re(0) = resid;
  accumulateTaggedLocalResidual();
}

void
FVInterfaceKernel::processJacobian(const ADReal & resid, const dof_id_type dof_index)
{
  _assembly.processJacobian(resid, dof_index, _matrix_tags);
}

void
FVInterfaceKernel::computeResidual(const FaceInfo & fi)
{
  setupData(fi);

  const auto var_elem_num = _elem_is_one ? _var1.number() : _var2.number();
  const auto var_neigh_num = _elem_is_one ? _var2.number() : _var1.number();

  const auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());

  processResidual(r, var_elem_num, false);
  processResidual(-r, var_neigh_num, true);
}

void
FVInterfaceKernel::computeResidualAndJacobian(const FaceInfo & fi)
{
  computeJacobian(fi);
}

void
FVInterfaceKernel::computeJacobian(const FaceInfo & fi)
{
  setupData(fi);

  const auto & elem_dof_indices = _elem_is_one ? _var1.dofIndices() : _var2.dofIndices();
  const auto & neigh_dof_indices =
      _elem_is_one ? _var2.dofIndicesNeighbor() : _var1.dofIndicesNeighbor();
  mooseAssert((elem_dof_indices.size() == 1) && (neigh_dof_indices.size() == 1),
              "We're currently built to use CONSTANT MONOMIALS");

  const auto r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

  _assembly.processResidualAndJacobian(r, elem_dof_indices[0], _vector_tags, _matrix_tags);
  _assembly.processResidualAndJacobian(-r, neigh_dof_indices[0], _vector_tags, _matrix_tags);
}

Moose::ElemArg
FVInterfaceKernel::elemArg(const bool correct_skewness) const
{
  return {_face_info->elemPtr(), correct_skewness};
}

Moose::ElemArg
FVInterfaceKernel::neighborArg(const bool correct_skewness) const
{
  return {_face_info->neighborPtr(), correct_skewness};
}

Moose::FaceArg
FVInterfaceKernel::singleSidedFaceArg(const MooseVariableFV<Real> & /*variable*/,
                                      const FaceInfo * fi,
                                      const Moose::FV::LimiterType limiter_type,
                                      const bool correct_skewness) const
{
  if (!fi)
    fi = _face_info;
  return makeFace(*fi, limiter_type, true, correct_skewness);
}

bool
FVInterfaceKernel::hasFaceSide(const FaceInfo &, bool) const
{
  // Our default interface kernel treats elem and neighbor sides equivalently so we will assume for
  // now that we will happily consume functor evaluations on either side of a face and any
  // interpolation between said evaluations
  return true;
}
