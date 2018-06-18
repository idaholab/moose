/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "CohesiveZoneMeshSplitBase.h"
#include "Parser.h"
#include "MooseUtils.h"
#include "Moose.h"
#include "MooseApp.h"
#include "MooseError.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/nemesis_io.h"
#include "libmesh/parallel_mesh.h"
#include "libmesh/serial_mesh.h"

template <>
InputParameters
validParams<CohesiveZoneMeshSplitBase>()
{
  InputParameters params = validParams<MooseMesh>();
  params.addRequiredParam<MeshFileName>("file", "The name of the mesh file");
  params.addParam<std::string>("interface_name",
                               "czm_interface",
                               "the name of"
                               "the new interface");
  params.addParam<BoundaryID>("interface_id",
                              100,
                              "the ID of the new"
                              "interface");
  params.addParam<bool>("split_interface",
                        false,
                        "If true, it create a "
                        "different interface for each block pair");
  params.addClassDescription("This is the base class used to split a monolithic"
                             "mesh by blocks pairs");
  return params;
}

CohesiveZoneMeshSplitBase::CohesiveZoneMeshSplitBase(const InputParameters & parameters)
  : MooseMesh(parameters),
    _file_name(getParam<MeshFileName>("file")),
    _interface_name(getParam<std::string>("interface_name")),
    _interface_id(getParam<BoundaryID>("interface_id")),
    _split_interface(getParam<bool>("split_interface"))
{
  getMesh().set_mesh_dimension(getParam<MooseEnum>("dim"));
}

CohesiveZoneMeshSplitBase::CohesiveZoneMeshSplitBase(const CohesiveZoneMeshSplitBase & other_mesh)
  : MooseMesh(other_mesh), _file_name(other_mesh._file_name)
{
}

CohesiveZoneMeshSplitBase::~CohesiveZoneMeshSplitBase() {}

std::unique_ptr<MooseMesh>
CohesiveZoneMeshSplitBase::safeClone() const
{
  return libmesh_make_unique<CohesiveZoneMeshSplitBase>(*this);
}

void
CohesiveZoneMeshSplitBase::init()
{
  mooseError("CohesiveZoneMeshSplitBase should never be called directly!"
             "Always use one of its the derived classes");
}

void
CohesiveZoneMeshSplitBase::checkInputParameter()
{
  // check input consistency
  if (getParam<bool>("split_interface") &&
      (_pars.isParamSetByUser("interface_id") || _pars.isParamSetByUser("interface_name")))
  {
    mooseError("if split_interface == true, interface_name and/or interface_id"
               " cannot be specified by the user");
  }
  else
  {
    // check that the provided interface_id is not already used
    const std::set<BoundaryID> & currentBoundaryIds = getBoundaryIDs();
    if (currentBoundaryIds.count(getParam<BoundaryID>("interface_id")) != 0)
      mooseError("CohesiveZoneMeshSplitBase::checkInputParameter. "
                 "A boundary with the same interface_id already exists "
                 "in the mesh. Please select a different interface_id.");
  }
}

BoundaryID
CohesiveZoneMeshSplitBase::findFreeBoundaryId()
{
  const std::set<BoundaryID> & currentBoundaryIds = getBoundaryIDs();
  bool freeBoundaryNotFound = true;
  BoundaryID freeId;
  for (freeId = 0; freeId < std::numeric_limits<BoundaryID>::max(); freeId++)
  {
    if (currentBoundaryIds.count(freeId) == 0)
    {
      // bid is not in the set, boundaryID is free
      freeBoundaryNotFound = false;
      break;
    }
  }

  if (freeBoundaryNotFound)
    mooseError("Too many boundaries. Maximum limit exceeded!");

  return freeId;
}

std::string
CohesiveZoneMeshSplitBase::generateBoundaryName(const subdomain_id_type & masterBlockID,
                                                const subdomain_id_type & slaveBlockID)
{
  return "czm_bM" + std::to_string(masterBlockID) + "_bS" + std::to_string(slaveBlockID);
}

void
CohesiveZoneMeshSplitBase::mapBoundaryIdAndBoundaryName(BoundaryID & boundaryID,
                                                        std::string & boundaryName)
{
  _czm_bName_bID_set.insert(std::pair<std::string, int>(boundaryName, boundaryID));
}

void
CohesiveZoneMeshSplitBase::findBoundaryNameAndInd(const subdomain_id_type & masterBlockID,
                                                  const subdomain_id_type & slaveBlockID,
                                                  std::string & boundaryName,
                                                  BoundaryID & boundaryID,
                                                  BoundaryInfo & boundary_info)
{
  // TODO this method should be multiprocessor and multithread safe
  // rememeber to check with the developer team

  // mpi barrier
  // first check which boundary name will be created
  boundaryName = generateBoundaryName(masterBlockID, slaveBlockID);

  // check if the boundary name already exist
  bool checkBoundaryAlreadyExist = false;
  for (auto b : _czm_bName_bID_set)
  {
    if (b.first.compare(boundaryName) == 0)
    {
      boundaryID = b.second;
      checkBoundaryAlreadyExist = true;
    }
  }

  if (checkBoundaryAlreadyExist)
  {
    // mpi barrier end
    return;
  }
  else
  {
    boundaryID = findFreeBoundaryId();
    mapBoundaryIdAndBoundaryName(boundaryID, boundaryName);

    // rename the boundary
    boundary_info.sideset_name(boundaryID) = boundaryName;

    // mpi broadcast boundary info and _czm_bName_bID_set

    // mpi barrier end
    return;
  }
}

void
CohesiveZoneMeshSplitBase::buildMesh()
{
  Moose::perf_log.push("Read Mesh", "Setup");
  if (_is_nemesis)
  {
    // Nemesis_IO only takes a reference to DistributedMesh, so we can't
    // be quite so short here.
    DistributedMesh & pmesh = cast_ref<DistributedMesh &>(getMesh());
    Nemesis_IO(pmesh).read(_file_name);

    getMesh().allow_renumbering(false);

    // Even if we want repartitioning when load balancing later, we'll
    // begin with the default partitioning defined by the Nemesis
    // file.
    bool skip_partitioning_later = getMesh().skip_partitioning();
    getMesh().skip_partitioning(true);
    getMesh().prepare_for_use();
    getMesh().skip_partitioning(skip_partitioning_later);
  }
  else // not reading Nemesis files
  {
    // See if the user has requested reading a solution from the file.
    // If so, we'll need to read the mesh with the exodus reader instead
    // of using mesh.read().  This will read the mesh on every processor

    if (_app.setFileRestart() && (_file_name.rfind(".exd") < _file_name.size() ||
                                  _file_name.rfind(".e") < _file_name.size()))
    {
      MooseUtils::checkFileReadable(_file_name);

      _exreader = libmesh_make_unique<ExodusII_IO>(getMesh());
      _exreader->read(_file_name);

      getMesh().allow_renumbering(false);
      getMesh().prepare_for_use();
    }
    else
    {
      auto slash_pos = _file_name.find_last_of("/");
      auto path = _file_name.substr(0, slash_pos);
      auto file = _file_name.substr(slash_pos + 1);

      bool restarting;
      // If we are reading a mesh while restarting, then we might have
      // a solution file that relies on that mesh partitioning and/or
      // numbering.  In that case, we need to turn off repartitioning
      // and renumbering, at least at first.
      if (file == "LATEST")
      {
        std::list<std::string> dir_list(1, path);
        std::list<std::string> files = MooseUtils::getFilesInDirs(dir_list);

        // Fill in the name of the LATEST file so we can open it and read it.
        _file_name = MooseUtils::getLatestMeshCheckpointFile(files);
        restarting = true;
      }
      else
        restarting = _file_name.rfind(".cpa") < _file_name.size() ||
                     _file_name.rfind(".cpr") < _file_name.size();

      const bool skip_partitioning_later = restarting && getMesh().skip_partitioning();
      const bool allow_renumbering_later = restarting && getMesh().allow_renumbering();

      if (restarting)
      {
        getMesh().skip_partitioning(true);
        getMesh().allow_renumbering(false);
      }

      MooseUtils::checkFileReadable(_file_name);
      getMesh().read(_file_name);

      if (restarting)
      {
        getMesh().allow_renumbering(allow_renumbering_later);
        getMesh().skip_partitioning(skip_partitioning_later);
      }
    }
  }

  Moose::perf_log.pop("Read Mesh", "Setup");
}

void
CohesiveZoneMeshSplitBase::read(const std::string & file_name)
{
  if (dynamic_cast<DistributedMesh *>(&getMesh()) && !_is_nemesis)
    getMesh().read(file_name, /*mesh_data=*/NULL, /*skip_renumber=*/false);
  else
    getMesh().read(file_name, /*mesh_data=*/NULL, /*skip_renumber=*/true);
}
