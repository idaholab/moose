//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshGenerator.h"
#include "CastUniquePointer.h"
#include "RestartableDataIO.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/face_quad4.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/mesh_communication.h"

registerMooseObject("MooseApp", FileMeshGenerator);

defineLegacyParams(FileMeshGenerator);

InputParameters
FileMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshFileName>("file", "The filename to read.");
  params.addParam<std::vector<std::string>>(
      "exodus_extra_element_integers",
      "The variable names in the mesh file for loading extra element integers");
  params.addParam<bool>("use_for_exodus_restart",
                        false,
                        "True to indicate that the mesh file this generator is reading can be used "
                        "for restarting variables");
  params.addParam<bool>(
      "has_fake_neighbors", false, "True if reading a broken with fake neighbors");
  params.addParam<FileName>("fake_neighbor_list_file_name",
                            "The file name containing the fake neighbor list will be saved");
  params.addClassDescription("Read a mesh from a file.");
  return params;
}

FileMeshGenerator::FileMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _file_name(getParam<MeshFileName>("file")),
    _has_fake_neighbors(getParam<bool>("has_fake_neighbors")),
    _fake_neighbor_list_file_name(
        _has_fake_neighbors ? getParam<FileName>("fake_neighbor_list_file_name") : "empty")
{
  if (_has_fake_neighbors && !parameters.isParamSetByUser("fake_neighbor_list_file_name"))
    paramError("fake_neighbor_list_file_name",
               "whe setting has_fake_neighbors=true, you als need to provide "
               "fake_neighbor_list_file_name ");

  if (_has_fake_neighbors)
    readFakeNeighborListFromFile();
}

std::unique_ptr<MeshBase>
FileMeshGenerator::generate()
{
  auto mesh = buildMeshBaseObject();

  if (_has_fake_neighbors)
  {
    // in this we need to do temporarly disable  paritioning and element removal
    mesh->allow_remote_element_removal(false);
    mesh->skip_partitioning(true);
  }

  bool exodus =
      _file_name.rfind(".exd") < _file_name.size() || _file_name.rfind(".e") < _file_name.size();
  bool has_exodus_integers = isParamValid("exodus_extra_element_integers");
  bool restart_exodus = (getParam<bool>("use_for_exodus_restart") && _app.getExodusFileRestart());
  if (exodus)
  {
    auto exreader = std::make_shared<ExodusII_IO>(*mesh);

    if (has_exodus_integers)
      exreader->set_extra_integer_vars(
          getParam<std::vector<std::string>>("exodus_extra_element_integers"));

    if (restart_exodus)
    {
      _app.setExReaderForRestart(std::move(exreader));
      exreader->read(_file_name);
      mesh->allow_renumbering(false);
    }
    else
    {
      if (mesh->processor_id() == 0)
        exreader->read(_file_name);
      MeshCommunication().broadcast(*mesh);
    }

    mesh->prepare_for_use();
  }
  else
  {
    if (_pars.isParamSetByUser("exodus_extra_element_integers"))
      mooseError("\"exodus_extra_element_integers\" should be given only for Exodus mesh files");
    if (_pars.isParamSetByUser("use_for_exodus_restart"))
      mooseError("\"use_for_exodus_restart\" should be given only for Exodus mesh files");

    // to support LATEST word for loading checkpoint files
    std::string file_name = MooseUtils::convertLatestCheckpoint(_file_name, false);

    mesh->read(file_name);

    // we also read declared mesh meta data here if there is meta data file
    RestartableDataIO restartable(_app);
    std::string fname = file_name + "/meta_data_mesh" + restartable.getRestartableDataExt();
    if (MooseUtils::pathExists(fname))
    {
      restartable.setErrorOnLoadWithDifferentNumberOfProcessors(false);
      // get reference to mesh meta data (created by MooseApp)
      auto & meta_data = _app.getRestartableDataMap(MooseApp::MESH_META_DATA);
      if (restartable.readRestartableDataHeaderFromFile(fname, false))
        restartable.readRestartableData(meta_data, DataNames());
    }
  }

  if (_has_fake_neighbors)
  {
    reassignFakeNeighbors(*mesh);
    mesh->skip_partitioning(false);
    // now we loop over all the elements and sides, search for fake neighbors and rrelink them
    for (auto elem : mesh->active_element_ptr_range())
    {
      std::cerr << "working on element " << elem->get_extra_integer(_integer_id) << "\n";
      for (unsigned int s = 0; s < elem->n_sides(); s++)
      {
        std::cerr << "  side  " << s << " elem ";
        Elem * neighbor = elem->neighbor_ptr(s);
        if (neighbor == nullptr)
          std::cerr << "null_ptr   ";
        else if (neighbor == remote_elem)
          std::cerr << "remote_elem";
        else
          std::cerr << neighbor->get_extra_integer(_integer_id);
        std::cerr << "\n";
      }
      std::cerr << "\n \n  ";
    }
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
FileMeshGenerator::reassignFakeNeighbors(MeshBase & mesh)
{
  /// check if we have the proper element integer in teh mesh
  const std::string _integer_name = "bmbb_element_id";
  if (!mesh.has_elem_integer(_integer_name))
    mooseError("FileMeshGenerator: Mesh does not have an element integer names as", _integer_name);
  _integer_id = mesh.get_elem_integer_index(_integer_name);

  // we start by generating a map between element integers and element ids
  for (const auto & elem : mesh.active_element_ptr_range())
    _uelemid_to_elemid[elem->get_extra_integer(_integer_id)] = elem->id();

  // now we loop over all the elements and sides, search for fake neighbors and rrelink them
  for (auto elem : mesh.active_element_ptr_range())
  {
    const unsigned int elem_integer = elem->get_extra_integer(_integer_id);
    for (unsigned int s = 0; s < elem->n_sides(); s++)
    {
      std::pair<unsigned int, unsigned int> elem_side = std::make_pair(elem_integer, s);
      bool neighbor_added = false;
      neighbor_added = assignFakeNeighbors(mesh, *elem, elem_side, _fake_neighbor_map);
      if (!neighbor_added)
        neighbor_added = assignFakeNeighbors(mesh, *elem, elem_side, _fake_neighbor_map_inverted);
    }
  }

  // and... we are done
}

bool
FileMeshGenerator::assignFakeNeighbors(
    MeshBase & mesh,
    Elem & elem,
    const std::pair<unsigned int, unsigned int> & elem_side,
    const std::map<std::pair<dof_id_type, unsigned int>, std::pair<dof_id_type, unsigned int>> &
        fake_neighbor_map) const
{
  bool neighbor_added = false;
  const auto & it_check = fake_neighbor_map.find(elem_side);
  if (it_check != fake_neighbor_map.end())
  {
    // if we are here we are going to add a neighbor one way or another
    neighbor_added = true;
    // let's get the neighbor from the map
    const unsigned int neighbor_integer = it_check->second.first;
    const dof_id_type neighbor_id = getRealIDFromInteger(neighbor_integer);
    if (neighbor_id != DofObject::invalid_id)
    {
      // the id is in the map, means this is an active element
      Elem * neighbor = mesh.elem_ptr(neighbor_id);
      elem.set_neighbor(elem_side.second, neighbor);
      neighbor->set_neighbor(it_check->second.second, &elem);
    }
    else // the neighbor is a remote element
      elem.set_neighbor(elem_side.second, const_cast<RemoteElem *>(remote_elem));
  }
  return neighbor_added;
}

dof_id_type
FileMeshGenerator::getRealIDFromInteger(const unsigned int elem_integer) const
{
  dof_id_type elem_id = DofObject::invalid_id;
  const auto & map_it = _uelemid_to_elemid.find(elem_integer);
  if (map_it != _uelemid_to_elemid.end())
    elem_id = map_it->second;
  return elem_id;
}

void
FileMeshGenerator::readFakeNeighborListFromFile()
{
  // Read in teh fake neighbor list
  std::ifstream inFile(_fake_neighbor_list_file_name.c_str());
  if (!inFile)
    mooseError("FileMeshGenerator can't open ", _fake_neighbor_list_file_name);

  // Skip first line
  for (unsigned int i = 0; i < 1; ++i)
    inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  // Loop over fake neighbors
  unsigned int e1, n1;
  unsigned int se1, sn1;
  char coma;

  while (inFile >> e1 >> coma >> se1 >> coma >> n1 >> coma >> sn1)
  {
    std::pair<unsigned int, unsigned int> epair = std::make_pair(e1, se1);
    std::pair<unsigned int, unsigned int> npair = std::make_pair(n1, sn1);

    _fake_neighbor_map[epair] = npair;
    _fake_neighbor_map_inverted[npair] = epair;
  }
}
