//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeriodicBCHelper.h"

#include "DisplacedProblem.h"
#include "FEProblemBase.h"
#include "FunctionPeriodicBoundary.h"
#include "MooseObjectAction.h"
#include "MooseMesh.h"

#include "libmesh/periodic_boundary.h"
#include "libmesh/default_coupling.h"

namespace Moose
{
InputParameters
PeriodicBCHelper::validParams()
{
  auto params = emptyInputParameters();

  const MultiMooseEnum auto_direction("x=0 y=1 z=2");
  params.addParam<MultiMooseEnum>(
      "auto_direction",
      auto_direction,
      "If using a generated mesh, the dimension(s) you want to mark as periodic.");

  params.addParam<BoundaryName>("primary", "Boundary associated with the primary boundary.");
  params.addParam<BoundaryName>("secondary", "Boundary associated with the secondary boundary.");
  params.addParam<RealVectorValue>("translation",
                                   "Vector that translates coordinates on the primary boundary to "
                                   "coordinates on the secondary boundary.");
  params.addParam<std::vector<std::string>>("transform_func",
                                            "Functions that specify the transformation");
  params.addParam<std::vector<std::string>>("inv_transform_func",
                                            "Functions that specify the inverse transformation");
  return params;
}

PeriodicBCHelper::PeriodicBCHelper(const Action & action) : _action(action), _params(getParams()) {}

void
PeriodicBCHelper::checkPeriodicParams() const
{
  if (_params.isParamValid("auto_direction"))
  {
    for (const auto & param :
         {"primary", "secondary", "translation", "transform_func", "inv_transform_func"})
      if (_params.isParamValid(param))
        paramError(param, "Should not be specified along with 'auto_direction'");
  }
  else if (!_params.isParamValid("primary") || !_params.isParamValid("secondary"))
    _action.mooseError(
        "Either 'auto_direction' or both 'primary' and 'secondary' must be specified");
}

void
PeriodicBCHelper::setupPeriodicBoundaries(FEProblemBase & problem)
{
  mooseAssert(_periodic_boundaries.empty(), "Already set");

  if (_params.isParamValid("auto_direction"))
    setupAutoPeriodicBoundaries(problem);
  else
    setupManualPeriodicBoundaries(problem);

  mooseAssert(getPeriodicBoundaries().size(), "Shouldn't run without boundaries");

  const auto add_ghosting = [this](auto & problem)
  {
    auto & mesh = problem.mesh().getMesh();
    auto functor = std::make_shared<libMesh::DefaultCoupling>();
    functor->set_mesh(&mesh);
    functor->set_n_levels(1);
    functor->set_periodic_boundaries(&getPeriodicBoundaries());
    mesh.add_ghosting_functor(functor);
  };

  add_ghosting(problem);
  if (const auto displaced_problem = problem.getDisplacedProblem())
    add_ghosting(*displaced_problem);
}

void
PeriodicBCHelper::addPeriodicBoundary(std::unique_ptr<libMesh::PeriodicBoundaryBase> p)
{
  onSetupPeriodicBoundary(*p);

  _periodic_boundaries.emplace(p->myboundary, p->clone());
  _periodic_boundaries.emplace(p->pairedboundary, p->clone(libMesh::PeriodicBoundaryBase::INVERSE));
}

void
PeriodicBCHelper::setupAutoPeriodicBoundaries(FEProblemBase & problem)
{
  auto & mesh = problem.mesh();

  // If we are working with a parallel mesh then we're going to ghost all the boundaries
  // everywhere because we don't know what we need...
  if (mesh.isDistributedMesh() && !mesh.detectOrthogonalDimRanges())
    _action.mooseError("Could not detect orthogonal dimension ranges for DistributedMesh.");

  for (const auto & dir : _params.get<MultiMooseEnum>("auto_direction"))
  {
    const int component = dir.id();
    if (component > (cast_int<int>(mesh.dimension()) - 1))
      paramError("auto_direction",
                 "Component '" + dir.name() + "' not valid for " +
                     std::to_string(mesh.dimension()) + "D mesh");

    const auto boundary_ids = mesh.getPairedBoundaryMapping(component);
    if (!boundary_ids)
      paramError("auto_direction",
                 "Couldn't auto-detect a paired boundary for use with periodic boundary "
                 "conditions in the '" +
                     dir.name() + "' direction");

    RealVectorValue v;
    v(component) = mesh.dimensionWidth(component);

    auto p = std::make_unique<libMesh::PeriodicBoundary>(v);
    p->myboundary = boundary_ids->first;
    p->pairedboundary = boundary_ids->second;

    addPeriodicBoundary(std::move(p));
  }
}

void
PeriodicBCHelper::setupManualPeriodicBoundaries(FEProblemBase & problem)
{
  auto & mesh = problem.mesh();
  std::unique_ptr<libMesh::PeriodicBoundaryBase> p;

  if (const auto translation_ptr = _params.queryParam<RealVectorValue>("translation"))
    p = std::make_unique<libMesh::PeriodicBoundary>(*translation_ptr);
  else if (const auto fn_names_ptr = _params.queryParam<std::vector<std::string>>("transform_func"))
  {
    const auto inv_fn_names_ptr =
        _params.queryParam<std::vector<std::string>>("inv_transform_func");
    if (!inv_fn_names_ptr)
      paramError("transform_func", "Must also specify 'inv_transform_func'");
    if (fn_names_ptr->size() != mesh.dimension())
      paramError("transform_func",
                 "Must be the size of the mesh dimension " + std::to_string(mesh.dimension()));
    if (inv_fn_names_ptr->size() != mesh.dimension())
      paramError("inv_transform_func",
                 "Must be the size of the mesh dimension " + std::to_string(mesh.dimension()));

    p = std::make_unique<FunctionPeriodicBoundary>(problem, *fn_names_ptr, *inv_fn_names_ptr);
  }
  else
    _action.mooseError(
        "You need to specify either 'auto_direction', 'translation', or 'transform_func'");

  const auto get_boundary = [this, &mesh](const auto & param) -> BoundaryID
  {
    if (const auto name_ptr = _params.queryParam<BoundaryName>(param))
    {
      if (const auto id = MooseMeshUtils::getBoundaryID(*name_ptr, mesh);
          id != Moose::INVALID_BOUNDARY_ID)
        return id;
      paramError(param, "Boundary '" + *name_ptr + "' does not exist in the mesh");
    }
    _action.mooseError("Parameter '", param, "' is required when 'auto_direction' is not set");
  };
  p->myboundary = get_boundary("primary");
  p->pairedboundary = get_boundary("secondary");

  addPeriodicBoundary(std::move(p));
}

void
PeriodicBCHelper::paramError(const std::string & param_name, const std::string & message) const
{
  // Eventually we should be able to do _params.paramError(), but that's not in yet
  mooseError(_params.errorPrefix(param_name), ": ", message);
}

const InputParameters &
PeriodicBCHelper::getParams() const
{
  if (const auto moa = dynamic_cast<const MooseObjectAction *>(&_action))
    return moa->getObjectParams();
  return _action.parameters();
}
}
