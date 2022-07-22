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
#include "MooseCoordTransform.h"

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
      "When false the check between the multiapp and transfer execute on flags is not preformed.");
  params.addParam<bool>("displaced_source_mesh",
                        false,
                        "Whether or not to use the displaced mesh for the source mesh.");
  params.addParam<bool>("displaced_target_mesh",
                        false,
                        "Whether or not to use the displaced mesh for the target mesh.");
  return params;
}

MultiAppTransfer::MultiAppTransfer(const InputParameters & parameters)
  : Transfer(parameters),
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
      _directions.push_back("from_multiapp");
    else if (_to_multi_app && (!_from_multi_app || _from_multi_app == _to_multi_app))
      _directions.push_back("to_multiapp");
    else
      _directions.push_back("between_multiapp");

    // So it's available in the next constructors
    _direction = _directions[0];
    _current_direction = _directions[0];
  }

  // Check for different number of subapps
  if (_to_multi_app && _from_multi_app &&
      _from_multi_app->numGlobalApps() != _to_multi_app->numGlobalApps())
    mooseError(
        "Between multiapp transfer is only supported with the same number of subapps per MultiApp");

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
  if (_from_multi_app)
    if (getExecuteOnEnum() != _from_multi_app->getExecuteOnEnum())
      mooseDoOnce(
          mooseWarning("MultiAppTransfer execute_on flags do not match associated from_multi_app "
                       "execute_on flags"));

  if (_to_multi_app)
    if (getExecuteOnEnum() != _to_multi_app->getExecuteOnEnum())
      mooseDoOnce(
          mooseWarning("MultiAppTransfer execute_on flags do not match associated to_multi_app "
                       "execute_on flags"));
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

const std::vector<ExecFlagType> &
MultiAppTransfer::execFlags() const
{
  mooseDeprecated(
      "The execFlags() methods is being removed because MOOSE has been updated to use a "
      "ExecFlagEnum for execute flags. The current flags should be retrieved from "
      "the \"exeucte_on\" parameters of your object or by using the \"_execute_enum\" "
      "reference to the parameter or the getExecuteOnEnum() method.");
  return _exec_flags;
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
    _to_transforms.push_back(&_from_multi_app->problemBase().coordTransform());

    getFromMultiAppInfo();
  }

  if (_current_direction == TO_MULTIAPP)
  {
    _from_problems.push_back(&_to_multi_app->problemBase());
    _from_positions.push_back(Point(0., 0., 0.));
    _from_transforms.push_back(&_to_multi_app->problemBase().coordTransform());

    getToMultiAppInfo();
  }

  if (_current_direction == BETWEEN_MULTIAPP)
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

  if (!_from_transforms.empty() && !_to_transforms.empty())
  {
    // set the destination coordinate systems for each transform for the purposes of determining
    // coordinate collapsing. For example if TO is XYZ and FROM is RZ, then TO will have its XYZ
    // coordinates collapsed into RZ and FROM will have a no-op for coordinate collapsing
    const auto & ex_from_transform = *_from_transforms[0];
    const auto & ex_to_transform = *_to_transforms[0];

    for (auto * const from_transform : _from_transforms)
      from_transform->setDestinationCoordinateSystem(ex_to_transform);
    for (auto * const to_transform : _to_transforms)
      to_transform->setDestinationCoordinateSystem(ex_from_transform);
  }
}

namespace
{
void
fillInfo(MultiApp & multi_app,
         std::vector<unsigned int> & map,
         std::vector<FEProblemBase *> & problems,
         std::vector<Point> & positions,
         std::vector<MooseCoordTransform *> & transforms)
{
  for (unsigned int i_app = 0; i_app < multi_app.numGlobalApps(); i_app++)
  {
    if (!multi_app.hasLocalApp(i_app))
      continue;

    auto & subapp_problem = multi_app.appProblemBase(i_app);
    const auto position = multi_app.position(i_app);
    auto & subapp_transform = subapp_problem.coordTransform();
    subapp_transform.setTranslationVector(position);

    map.push_back(i_app);
    problems.push_back(&subapp_problem);
    positions.push_back(position);
    transforms.push_back(&subapp_transform);
  }

  if (transforms.empty())
    // Can happen if there are no local apps
    return;

  const auto first_transform = transforms[0]->coordinateSystem();
  std::for_each(transforms.begin() + 1,
                transforms.end(),
                [first_transform](const auto * const transform_obj)
                {
                  if (transform_obj->coordinateSystem() != first_transform)
                    mooseError("Coordinate systems must be consistent between multiapps");
                });
}
}

void
MultiAppTransfer::getToMultiAppInfo()
{
  if (!_to_multi_app)
    mooseError("There is no to_multiapp to get info from");

  fillInfo(*_to_multi_app, _to_local2global_map, _to_problems, _to_positions, _to_transforms);
}

void
MultiAppTransfer::getFromMultiAppInfo()
{
  if (!_from_multi_app)
    mooseError("There is no from_multiapp to get info from");

  fillInfo(
      *_from_multi_app, _from_local2global_map, _from_problems, _from_positions, _from_transforms);
}

namespace
{
void
transformBoundingBox(BoundingBox & box, const MooseCoordTransform & transform)
{
  const Real min_x = box.first(0);
  const Real max_x = box.second(0);
  const Real min_y = box.first(1);
  const Real max_y = box.second(1);
  const Real min_z = box.first(2);
  const Real max_z = box.second(2);

  std::array<Point, 8> box_corners = {{Point(min_x, min_y, min_z),
                                       Point(max_x, min_y, min_z),
                                       Point(min_x, max_y, min_z),
                                       Point(max_x, max_y, min_z),
                                       Point(min_x, min_y, max_z),
                                       Point(max_x, min_y, max_z),
                                       Point(min_x, max_y, max_z),
                                       Point(max_x, max_y, max_z)}};

  // transform each corner
  for (auto & corner : box_corners)
    corner = transform(corner);

  // Create new bounding box
  Point new_box_min = box_corners[0];
  Point new_box_max = new_box_min;
  for (const auto p : make_range(1, 8))
    for (const auto d : make_range(Moose::dim))
    {
      const Point & pt = box_corners[p];
      if (new_box_min(d) > pt(d))
        new_box_min(d) = pt(d);

      if (new_box_max(d) < pt(d))
        new_box_max(d) = pt(d);
    }
  box.first = new_box_min;
  box.second = new_box_max;
}

template <typename T>
void
extendBoundingBoxes(const Real factor, T & bboxes)
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
    transformBoundingBox(bbox, *_from_transforms[i]);

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
      // Translate the bounding box to the from domain's position. We may have rotations so we must
      // be careful in constructing the new min and max (first and second)
      transformBoundingBox(bbox, *_from_transforms[i]);

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
