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

defineLegacyParams(MultiAppTransfer);

InputParameters
MultiAppTransfer::validParams()
{
  InputParameters params = Transfer::validParams();
  params.addRequiredParam<MultiAppName>("multi_app", "The name of the MultiApp to use.");

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
    _multi_app(_fe_problem.getMultiApp(getParam<MultiAppName>("multi_app"))),
    _displaced_source_mesh(getParam<bool>("displaced_source_mesh")),
    _displaced_target_mesh(getParam<bool>("displaced_target_mesh"))
{
  if (getParam<bool>("check_multiapp_execute_on"))
    checkMultiAppExecuteOn();

  if (_directions.size() == 0)
    paramError("direction", "At least one direction is required");

  // If we have just one direction in _directions, set it now so that it can be used in the
  // constructor
  if (_directions.size() == 1)
  {
    int only_direction = _directions.get(0);
    _current_direction = only_direction;
    _direction = only_direction;
  }
}

void
MultiAppTransfer::checkMultiAppExecuteOn()
{
  if (getExecuteOnEnum() != _multi_app->getExecuteOnEnum())
    mooseDoOnce(mooseWarning("MultiAppTransfer execute_on flags do not match associated Multiapp "
                             "execute_on flags"));
}

void
MultiAppTransfer::variableIntegrityCheck(const AuxVariableName & var_name) const
{
  for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
    if (_multi_app->hasLocalApp(i) && !_multi_app->appProblemBase(i).hasVariable(var_name))
      mooseError("Cannot find variable ", var_name, " for ", name(), " Transfer");
}

const std::vector<ExecFlagType> &
MultiAppTransfer::execFlags() const
{
  mooseDeprecated("The execFlags() methos is being removed because MOOSE has been updated to use a "
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

  // Build the vectors for to problems, from problems, and subapps positions.
  switch (_direction)
  {
    case TO_MULTIAPP:
      _from_problems.push_back(&_multi_app->problemBase());
      _from_positions.push_back(Point(0., 0., 0.));
      for (unsigned int i_app = 0; i_app < _multi_app->numGlobalApps(); i_app++)
      {
        if (!_multi_app->hasLocalApp(i_app))
          continue;
        _local2global_map.push_back(i_app);
        _to_problems.push_back(&_multi_app->appProblemBase(i_app));
        _to_positions.push_back(_multi_app->position(i_app));
      }
      break;

    case FROM_MULTIAPP:
      _to_problems.push_back(&_multi_app->problemBase());
      _to_positions.push_back(Point(0., 0., 0.));
      for (unsigned int i_app = 0; i_app < _multi_app->numGlobalApps(); i_app++)
      {
        if (!_multi_app->hasLocalApp(i_app))
          continue;
        _local2global_map.push_back(i_app);
        _from_problems.push_back(&_multi_app->appProblemBase(i_app));
        _from_positions.push_back(_multi_app->position(i_app));
      }
      break;
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

    // Translate the bounding box to the from domain's position.
    bbox.first += _from_positions[i];
    bbox.second += _from_positions[i];

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
      if (bnode->_bnd_id == boundary_id && bnode->_node->processor_id() == processor_id())
      {
        at_least_one = true;
        const auto & node = *bnode->_node;
        for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
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
      // Translate the bounding box to the from domain's position.
      bbox.first += _from_positions[i];
      bbox.second += _from_positions[i];
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

  return bboxes;
}

std::vector<unsigned int>
MultiAppTransfer::getFromsPerProc()
{
  std::vector<unsigned int> froms_per_proc;
  switch (_direction)
  {
    case TO_MULTIAPP:
      froms_per_proc.resize(n_processors(), 1);
      break;
    case FROM_MULTIAPP:
      froms_per_proc.resize(n_processors());
      _communicator.allgather(_multi_app->numLocalApps(), froms_per_proc);
      break;
  }
  return froms_per_proc;
}

NumericVector<Real> &
MultiAppTransfer::getTransferVector(unsigned int i_local, std::string var_name)
{
  mooseAssert(_direction == TO_MULTIAPP, "getTransferVector only works for transfers to multiapps");

  return _multi_app->appTransferVector(_local2global_map[i_local], var_name);
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
