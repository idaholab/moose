//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MultiAppTransfer.h"
#include "Transfer.h"
#include "MooseTypes.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "MultiApp.h"
#include "MooseMesh.h"

#include "libmesh/parallel_algebra.h"
#include "libmesh/mesh_tools.h"

InputParameters
MultiAppTransfer::validParams()
{
  InputParameters params = Transfer::validParams();
  params.addDeprecatedParam<MultiAppName>("multi_app",
                                          "The name of the MultiApp to transfer data with",
                                          "Use to_multiapp & from_multiapp parameters now");
  params.addParam<MultiAppName>("from_multi_app", "The name of the MultiApp to receive data from");
  params.addParam<MultiAppName>("to_multi_app", "The name of the MultiApp to transfer the data to");

  // MultiAppTransfers by default will execute with their associated MultiApp. These flags will be
  // added by FEProblemBase when the transfer is added.
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_SAME_AS_MULTIAPP);
  exec_enum = EXEC_SAME_AS_MULTIAPP;
  params.setDocString("execute_on", exec_enum.getDocString());

  params.addParam<bool>(
      "check_multiapp_execute_on",
      true,
      "When false the check between the multiapp and transfer execute on flags is not performed.");
  params.addParam<bool>("displaced_source_mesh",
                        false,
                        "Whether or not to use the displaced mesh for the source mesh.");
  params.addParam<bool>("displaced_target_mesh",
                        false,
                        "Whether or not to use the displaced mesh for the target mesh.");
  addSkipCoordCollapsingParam(params);
  return params;
}

void
MultiAppTransfer::addBBoxFactorParam(InputParameters & params)
{
  params.addRangeCheckedParam<Real>(
      "bbox_factor",
      1 + TOLERANCE,
      "bbox_factor>0",
      "Multiply bounding box width (in all directions) by the prescribed factor. Values less than "
      "1 will shrink the bounding box; values greater than 1 will enlarge the bounding box. It is "
      "generally not advised to ever shrink the bounding box. On the other hand it may be helpful "
      "to enlarge the bounding box. Larger bounding boxes will lead to more accurate determination "
      "of the closest node/element with the tradeoff of more communication.");
}

void
MultiAppTransfer::addSkipCoordCollapsingParam(InputParameters & params)
{
  params.addParam<bool>(
      "skip_coordinate_collapsing",
      true,
      "Whether to skip coordinate collapsing (translation and rotation are still performed, only "
      "XYZ, RZ etc collapsing is skipped) when performing mapping and inverse "
      "mapping coordinate transformation operations. This parameter should only "
      "be set by users who really know what they're doing.");
  params.addParamNamesToGroup("skip_coordinate_collapsing", "Advanced");
}

MultiAppTransfer::MultiAppTransfer(const InputParameters & parameters)
  : Transfer(parameters),
    _skip_coordinate_collapsing(getParam<bool>("skip_coordinate_collapsing")),
    _displaced_source_mesh(getParam<bool>("displaced_source_mesh")),
    _displaced_target_mesh(getParam<bool>("displaced_target_mesh")),
    _bbox_factor(isParamValid("bbox_factor") ? getParam<Real>("bbox_factor") : 1)
{
  // Get the multiapps from their names
  if (!isParamValid("multi_app"))
  {
    if (isParamValid("from_multi_app"))
    {
      _from_multi_app = _fe_problem.getMultiApp(getParam<MultiAppName>("from_multi_app"));
      _multi_app = _from_multi_app;
    }
    if (isParamValid("to_multi_app"))
    {
      _to_multi_app = _fe_problem.getMultiApp(getParam<MultiAppName>("to_multi_app"));
      _multi_app = _to_multi_app;
    }
    if (!isParamValid("direction") && !isParamValid("from_multi_app") &&
        !isParamValid("to_multi_app"))
      mooseError("from_multi_app and/or to_multi_app must be specified");
  }
  else
  {
    // Check deprecated direction parameter
    for (const auto & dir : _directions)
    {
      if (dir == FROM_MULTIAPP)
      {
        _from_multi_app = _fe_problem.getMultiApp(getParam<MultiAppName>("multi_app"));
        _multi_app = _from_multi_app;
      }
      else if (dir == TO_MULTIAPP)
      {
        _to_multi_app = _fe_problem.getMultiApp(getParam<MultiAppName>("multi_app"));
        _multi_app = _to_multi_app;
      }
      else
        paramError("direction",
                   "BETWEN_MULTIAPP transfers should be specified using to/from_multi_app");
    }
  }

  if (getParam<bool>("check_multiapp_execute_on"))
    checkMultiAppExecuteOn();

  // Fill direction attributes, for backward compatibility but also convenience
  if (!isParamValid("direction"))
  {
    if (_from_multi_app && (!_to_multi_app || _from_multi_app == _to_multi_app))
      _directions.setAdditionalValue("from_multiapp");
    if (_to_multi_app && (!_from_multi_app || _from_multi_app == _to_multi_app))
      _directions.setAdditionalValue("to_multiapp");
    if (_from_multi_app && _to_multi_app && _from_multi_app != _to_multi_app)
      _directions.setAdditionalValue("between_multiapp");

    // So it's available in the next constructors
    _direction = _directions[0];
    _current_direction = _directions[0];
  }

  // Handle deprecated parameters
  if (parameters.isParamSetByUser("direction"))
  {
    if (!isParamValid("multi_app"))
      paramError("direction",
                 "The deprecated 'direction' parameter is meant to be used in conjunction with the "
                 "'multi_app' parameter");
    if (isParamValid("to_multi_app") || isParamValid("from_multi_app"))
      paramError("direction",
                 "The deprecated 'direction' parameter is not meant to be used in conjunction with "
                 "the 'from_multi_app' or 'to_multi_app' parameters");
  }
}

void
MultiAppTransfer::checkMultiAppExecuteOn()
{
  if (_from_multi_app && !_to_multi_app)
    if (getExecuteOnEnum() != _from_multi_app->getExecuteOnEnum())
      mooseDoOnce(
          mooseWarning("MultiAppTransfer execute_on flags do not match associated from_multi_app "
                       "execute_on flags"));

  if (_to_multi_app && !_from_multi_app)
    if (getExecuteOnEnum() != _to_multi_app->getExecuteOnEnum())
      mooseDoOnce(
          mooseWarning("MultiAppTransfer execute_on flags do not match associated to_multi_app "
                       "execute_on flags"));

  // In the case of siblings transfer, the check will be looser
  if (_from_multi_app && _to_multi_app)
    if (getExecuteOnEnum() != _from_multi_app->getExecuteOnEnum() &&
        getExecuteOnEnum() != _to_multi_app->getExecuteOnEnum())
      mooseDoOnce(
          mooseWarning("MultiAppTransfer execute_on flags do not match associated to_multi_app "
                       "and from_multi_app execute_on flags"));
}

void
MultiAppTransfer::variableIntegrityCheck(const AuxVariableName & var_name) const
{
  bool variable_found = false;
  bool has_an_app = false;

  // Check the from_multi_app for the variable
  if (_from_multi_app)
    for (unsigned int i = 0; i < _from_multi_app->numGlobalApps(); i++)
      if (_from_multi_app->hasLocalApp(i))
      {
        has_an_app = true;
        if (_from_multi_app->appProblemBase(i).hasVariable(var_name))
          variable_found = true;
      }

  // Check the to_multi_app for the variable
  if (_to_multi_app)
    for (unsigned int i = 0; i < _to_multi_app->numGlobalApps(); i++)
      if (_to_multi_app->hasLocalApp(i))
      {
        has_an_app = true;
        if (_to_multi_app->appProblemBase(i).hasVariable(var_name))
          variable_found = true;
      }

  if (!variable_found && has_an_app)
    mooseError("Cannot find variable ", var_name, " for ", name(), " Transfer");
}

void
MultiAppTransfer::initialSetup()
{
  // Check for siblings transfer support
  if (_to_multi_app && _from_multi_app)
    checkSiblingsTransferSupported();

  getAppInfo();

  if (_from_multi_app)
    _from_multi_app->addAssociatedTransfer(*this);
  if (_to_multi_app)
    _to_multi_app->addAssociatedTransfer(*this);
}

void
MultiAppTransfer::getAppInfo()
{
  // I would like to do all of this in initialSetup, but it will fail with
  // multiapps that reset.  A reset deletes and rebuilds the FEProblems so all
  // of the pointers will be broken.

  // Clear the vectors since we've probably built them up from a previous call
  _from_problems.clear();
  _to_problems.clear();
  _from_es.clear();
  _to_es.clear();
  _from_meshes.clear();
  _to_meshes.clear();
  _to_positions.clear();
  _from_positions.clear();
  _to_transforms.clear();
  _from_transforms.clear();
  // Clear this map since we build it from scratch every time we transfer
  // Otherwise, this will cause two issues: 1) increasing memory usage
  // for a simulation that requires many transfers, 2) producing wrong results
  // when we do collective communication on this vector.
  _to_local2global_map.clear();
  _from_local2global_map.clear();

  // Build the vectors for to problems, from problems, and subapps positions.
  if (_current_direction == FROM_MULTIAPP)
  {
    _to_problems.push_back(&_from_multi_app->problemBase());
    _to_positions.push_back(Point(0., 0., 0.));
    getFromMultiAppInfo();
  }
  else if (_current_direction == TO_MULTIAPP)
  {
    _from_problems.push_back(&_to_multi_app->problemBase());
    _from_positions.push_back(Point(0., 0., 0.));
    getToMultiAppInfo();
  }
  else if (_current_direction == BETWEEN_MULTIAPP)
  {
    mooseAssert(&_from_multi_app->problemBase().coordTransform() ==
                    &_to_multi_app->problemBase().coordTransform(),
                "I believe these should be the same. If not, then it will be difficult to define a "
                "canonical reference frame.");
    getToMultiAppInfo();
    getFromMultiAppInfo();
  }

  // Build the from and to equation systems and mesh vectors.
  for (unsigned int i = 0; i < _to_problems.size(); i++)
  {
    // TODO: Do I actually want es or displaced es?
    _to_es.push_back(&_to_problems[i]->es());
    if (_displaced_target_mesh && _to_problems[i]->getDisplacedProblem())
      _to_meshes.push_back(&_to_problems[i]->getDisplacedProblem()->mesh());
    else
      _to_meshes.push_back(&_to_problems[i]->mesh());
  }

  for (unsigned int i = 0; i < _from_problems.size(); i++)
  {
    _from_es.push_back(&_from_problems[i]->es());
    if (_displaced_source_mesh && _from_problems[i]->getDisplacedProblem())
      _from_meshes.push_back(&_from_problems[i]->getDisplacedProblem()->mesh());
    else
      _from_meshes.push_back(&_from_problems[i]->mesh());
  }

  MooseAppCoordTransform::MinimalData from_app_transform_construction_data{};
  if (_communicator.rank() == 0)
    from_app_transform_construction_data =
        _current_direction == TO_MULTIAPP
            ? _to_multi_app->problemBase().coordTransform().minimalDataDescription()
            : _from_multi_app->appProblemBase(0).coordTransform().minimalDataDescription();
  _communicator.broadcast(from_app_transform_construction_data);
  _from_moose_app_transform =
      std::make_unique<MooseAppCoordTransform>(from_app_transform_construction_data);

  MooseAppCoordTransform::MinimalData to_app_transform_construction_data{};
  if (_communicator.rank() == 0)
    to_app_transform_construction_data =
        _current_direction == FROM_MULTIAPP
            ? _from_multi_app->problemBase().coordTransform().minimalDataDescription()
            : _to_multi_app->appProblemBase(0).coordTransform().minimalDataDescription();
  _communicator.broadcast(to_app_transform_construction_data);
  _to_moose_app_transform =
      std::make_unique<MooseAppCoordTransform>(to_app_transform_construction_data);

  /*
   * skip_coordinate_collapsing: whether to set the transform to skip coordinate collapsing
   *                             (from XYZ to RZ for example)
   * transforms: vector of transforms to add the new transforms to
   * moose_app_transform: base for the new transform
   * is_parent_app_transform: whether working on the transform for the parent app (this app, the
   *                          one creating the transfer) or for child apps
   * multiapp: pointer to the multiapp to obtain the position of the child apps
   */
  auto create_multiapp_transforms = [this](auto & transforms,
                                           const auto & moose_app_transform,
                                           const bool is_parent_app_transform,
                                           const MultiApp * const multiapp = nullptr)
  {
    mooseAssert(is_parent_app_transform || multiapp,
                "Coordinate transform must be created either for child app or parent app");
    if (is_parent_app_transform)
    {
      transforms.push_back(std::make_unique<MultiAppCoordTransform>(moose_app_transform));
      transforms.back()->skipCoordinateCollapsing(_skip_coordinate_collapsing);
      // zero translation
    }
    else
    {
      mooseAssert(transforms.size() == 0, "transforms should not be initialized at this point");
      for (const auto i : make_range(multiapp->numGlobalApps()))
      {
        transforms.push_back(std::make_unique<MultiAppCoordTransform>(moose_app_transform));
        auto & transform = transforms[i];
        transform->skipCoordinateCollapsing(_skip_coordinate_collapsing);
        if (multiapp->usingPositions())
          transform->setTranslationVector(multiapp->position(i));
      }
    }
  };

  if (_current_direction == TO_MULTIAPP)
  {
    create_multiapp_transforms(
        _to_transforms, *_to_moose_app_transform, false, _to_multi_app.get());
    create_multiapp_transforms(_from_transforms, *_from_moose_app_transform, true);
  }
  if (_current_direction == FROM_MULTIAPP)
  {
    create_multiapp_transforms(_to_transforms, *_to_moose_app_transform, true);
    create_multiapp_transforms(
        _from_transforms, *_from_moose_app_transform, false, _from_multi_app.get());
  }
  if (_current_direction == BETWEEN_MULTIAPP)
  {
    create_multiapp_transforms(
        _to_transforms, *_to_moose_app_transform, false, _to_multi_app.get());
    create_multiapp_transforms(
        _from_transforms, *_from_moose_app_transform, false, _from_multi_app.get());
  }

  auto check_transform_compatibility = [this](const MultiAppCoordTransform & transform)
  {
    if (transform.hasNonTranslationTransformation() && !usesMooseAppCoordTransform())
      mooseWarning("Transfer '",
                   name(),
                   "' of type '",
                   type(),
                   "' has non-translation transformations but it does not implement coordinate "
                   "transformations using the 'MooseAppCoordTransform' class. Your data transfers "
                   "will not be performed in the expected transformed frame");
  };

  // set the destination coordinate systems for each transform for the purposes of determining
  // coordinate collapsing. For example if TO is XYZ and FROM is RZ, then TO will have its XYZ
  // coordinates collapsed into RZ and FROM will have a no-op for coordinate collapsing

  for (const auto i : index_range(_from_transforms))
  {
    auto & from_transform = _from_transforms[i];
    from_transform->setDestinationCoordTransform(*_to_moose_app_transform);
    if (i == 0)
      check_transform_compatibility(*from_transform);
  }
  for (const auto i : index_range(_to_transforms))
  {
    auto & to_transform = _to_transforms[i];
    to_transform->setDestinationCoordTransform(*_from_moose_app_transform);
    if (i == 0)
      check_transform_compatibility(*to_transform);
  }
}

namespace
{
void
fillInfo(MultiApp & multi_app,
         std::vector<unsigned int> & map,
         std::vector<FEProblemBase *> & problems,
         std::vector<Point> & positions)
{
  for (unsigned int i_app = 0; i_app < multi_app.numGlobalApps(); i_app++)
  {
    if (!multi_app.hasLocalApp(i_app))
      continue;

    auto & subapp_problem = multi_app.appProblemBase(i_app);

    map.push_back(i_app);
    problems.push_back(&subapp_problem);
    if (multi_app.usingPositions())
      positions.push_back(multi_app.position(i_app));
  }
}
}

void
MultiAppTransfer::getToMultiAppInfo()
{
  if (!_to_multi_app)
    mooseError("There is no to_multiapp to get info from");

  fillInfo(*_to_multi_app, _to_local2global_map, _to_problems, _to_positions);
}

void
MultiAppTransfer::getFromMultiAppInfo()
{
  if (!_from_multi_app)
    mooseError("There is no from_multiapp to get info from");

  fillInfo(*_from_multi_app, _from_local2global_map, _from_problems, _from_positions);
}

void
MultiAppTransfer::transformBoundingBox(BoundingBox & box, const MultiAppCoordTransform & transform)
{
  MultiApp::transformBoundingBox(box, transform);
}

void
MultiAppTransfer::extendBoundingBoxes(const Real factor, std::vector<BoundingBox> & bboxes) const
{
  const auto extension_factor = factor - 1;

  // Extend (or contract if the extension factor is negative) bounding boxes along all the
  // directions by the same length. Greater than zero values of this member may be necessary because
  // the nearest bounding box does not necessarily give you the closest node/element. It will depend
  // on the partition and geometry. A node/element will more likely find its nearest source
  // element/node by extending bounding boxes. If each of the bounding boxes covers the entire
  // domain, a node/element will be able to find its nearest source element/node for sure, but at
  // the same time, more communication will be involved and can be expensive.
  for (auto & box : bboxes)
  {
    // libmesh set an invalid bounding box using this code
    // for (unsigned int i=0; i<LIBMESH_DIM; i++)
    // {
    //   this->first(i)  =  std::numeric_limits<Real>::max();
    //   this->second(i) = -std::numeric_limits<Real>::max();
    // }
    // If it is an invalid box, we should skip it
    if (box.first(0) == std::numeric_limits<Real>::max())
      continue;

    auto width = box.second - box.first;
    box.second += width * extension_factor;
    box.first -= width * extension_factor;
  }
}

std::vector<BoundingBox>
MultiAppTransfer::getFromBoundingBoxes()
{
  std::vector<std::pair<Point, Point>> bb_points(_from_meshes.size());
  for (unsigned int i = 0; i < _from_meshes.size(); i++)
  {
    // Get a bounding box around the mesh elements that are local to the current
    // processor.
    BoundingBox bbox = MeshTools::create_local_bounding_box(*_from_meshes[i]);

    // Translate the bounding box to the from domain's position. We may have rotations so we must
    // be careful in constructing the new min and max (first and second)
    const auto from_global_num = getGlobalSourceAppIndex(i);
    transformBoundingBox(bbox, *_from_transforms[from_global_num]);

    // Cast the bounding box into a pair of points (so it can be put through
    // MPI communication).
    bb_points[i] = static_cast<std::pair<Point, Point>>(bbox);
  }

  // Serialize the bounding box points.
  _communicator.allgather(bb_points);

  // Recast the points back into bounding boxes and return.
  std::vector<BoundingBox> bboxes(bb_points.size());
  for (unsigned int i = 0; i < bb_points.size(); i++)
    bboxes[i] = static_cast<BoundingBox>(bb_points[i]);

  // possibly extend bounding boxes
  extendBoundingBoxes(_bbox_factor, bboxes);

  return bboxes;
}

std::vector<BoundingBox>
MultiAppTransfer::getFromBoundingBoxes(BoundaryID boundary_id)
{
  std::vector<std::pair<Point, Point>> bb_points(_from_meshes.size());
  const Real min_r = std::numeric_limits<Real>::lowest();
  const Real max_r = std::numeric_limits<Real>::max();

  for (unsigned int i = 0; i < _from_meshes.size(); i++)
  {

    Point min(max_r, max_r, max_r);
    Point max(min_r, min_r, min_r);
    bool at_least_one = false;

    // TODO: Factor this into mesh_tools after adding new boundary bounding box routine.
    const ConstBndNodeRange & bnd_nodes = *_from_meshes[i]->getBoundaryNodeRange();
    for (const auto & bnode : bnd_nodes)
    {
      if (bnode->_bnd_id == boundary_id &&
          bnode->_node->processor_id() == _from_meshes[i]->processor_id())
      {
        at_least_one = true;
        const auto & node = *bnode->_node;
        for (const auto i : make_range(Moose::dim))
        {
          min(i) = std::min(min(i), node(i));
          max(i) = std::max(max(i), node(i));
        }
      }
    }

    BoundingBox bbox(min, max);
    if (!at_least_one)
      bbox.min() = max; // If we didn't hit any nodes, this will be _the_ minimum bbox
    else
    {
      // Translate the bounding box to the from domain's position. We may have rotations so we must
      // be careful in constructing the new min and max (first and second)
      const auto from_global_num = getGlobalSourceAppIndex(i);
      transformBoundingBox(bbox, *_from_transforms[from_global_num]);
    }

    // Cast the bounding box into a pair of points (so it can be put through
    // MPI communication).
    bb_points[i] = static_cast<std::pair<Point, Point>>(bbox);
  }

  // Serialize the bounding box points.
  _communicator.allgather(bb_points);

  // Recast the points back into bounding boxes and return.
  std::vector<BoundingBox> bboxes(bb_points.size());
  for (unsigned int i = 0; i < bb_points.size(); i++)
    bboxes[i] = static_cast<BoundingBox>(bb_points[i]);

  // possibly extend bounding boxes
  extendBoundingBoxes(_bbox_factor, bboxes);

  return bboxes;
}

std::vector<unsigned int>
MultiAppTransfer::getFromsPerProc()
{
  std::vector<unsigned int> froms_per_proc;
  if (_to_multi_app)
    froms_per_proc.resize(n_processors(), 1);
  if (_from_multi_app)
  {
    froms_per_proc.resize(n_processors());
    _communicator.allgather(_from_multi_app->numLocalApps(), froms_per_proc);
  }
  return froms_per_proc;
}

NumericVector<Real> &
MultiAppTransfer::getTransferVector(unsigned int i_local, std::string var_name)
{
  mooseAssert(_to_multi_app, "getTransferVector only works for transfers to multiapps");

  return _to_multi_app->appTransferVector(_to_local2global_map[i_local], var_name);
}

void
MultiAppTransfer::checkVariable(const FEProblemBase & fe_problem,
                                const VariableName & var_name,
                                const std::string & param_name) const
{
  if (!fe_problem.hasVariable(var_name))
  {
    if (param_name.empty())
      mooseError("The variable '", var_name, "' does not exist.");
    else
      paramError(param_name, "The variable '", var_name, "' does not exist.");
  }
}

Point
MultiAppTransfer::getPointInTargetAppFrame(const Point & p,
                                           unsigned int local_i_to,
                                           const std::string & phase) const
{
  const auto & to_transform = _to_transforms[getGlobalTargetAppIndex(local_i_to)];
  if (to_transform->hasCoordinateSystemTypeChange())
  {
    if (!_skip_coordinate_collapsing)
      mooseInfo(phase + " cannot use the point in the target app frame due to the "
                        "non-uniqueness of the coordinate collapsing reverse mapping."
                        " Coordinate collapse is ignored for this operation");
    to_transform->skipCoordinateCollapsing(true);
    const auto target_point = to_transform->mapBack(p);
    to_transform->skipCoordinateCollapsing(false);
    return target_point;
  }
  else
    return to_transform->mapBack(p);
}

unsigned int
MultiAppTransfer::getGlobalSourceAppIndex(unsigned int i_from) const
{
  mooseAssert(_current_direction == TO_MULTIAPP || i_from < _from_local2global_map.size(),
              "Out of bounds local from-app index");
  return _current_direction == TO_MULTIAPP ? 0 : _from_local2global_map[i_from];
}

unsigned int
MultiAppTransfer::getGlobalTargetAppIndex(unsigned int i_to) const
{
  mooseAssert(_current_direction == FROM_MULTIAPP || i_to < _to_local2global_map.size(),
              "Out of bounds local to-app index");
  return _current_direction == FROM_MULTIAPP ? 0 : _to_local2global_map[i_to];
}

unsigned int
MultiAppTransfer::getLocalSourceAppIndex(unsigned int i_from) const
{
  return _current_direction == TO_MULTIAPP
             ? 0
             : _from_local2global_map[i_from] - _from_local2global_map[0];
}
