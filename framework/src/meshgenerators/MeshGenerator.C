//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshGenerator.h"
#include "MooseMesh.h"
#include "MooseApp.h"

#include "Exodus.h"
#include "Nemesis.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/nemesis_io.h"

InputParameters
MeshGenerator::validParams()
{
  InputParameters params = MooseObject::validParams();

  params.addParam<bool>("show_info",
                        false,
                        "Whether or not to show mesh info after generating the mesh "
                        "(bounding box, element types, sidesets, nodesets, subdomains, etc)");
  params.addParam<std::string>(
      "save_with_name",
      std::string(),
      "Keep the mesh from this mesh generator in memory with the name specified");

  params.addParam<bool>(
      "output", false, "Whether or not to output the mesh file after generating the mesh");
  params.addParam<bool>("nemesis",
                        false,
                        "Whether or not to output the mesh file in the nemesis"
                        "format (only if output = true)");

  params.addParamNamesToGroup("show_info output nemesis", "Debugging");
  params.addParamNamesToGroup("save_with_name", "Advanced");
  params.registerBase("MeshGenerator");

  return params;
}

MeshGenerator::MeshGenerator(const InputParameters & parameters)
  : MooseObject(parameters),
    MeshMetaDataInterface(this),
    _mesh(_app.actionWarehouse().mesh()),
    _save_with_name(getParam<std::string>("save_with_name"))
{
  if (_save_with_name == _app.getMeshGeneratorSystem().mainMeshGeneratorName())
    paramError(
        "save_with_name", "The user-defined mesh name: '", _save_with_name, "' is a reserved name");
}

const MeshGeneratorName *
MeshGenerator::getMeshGeneratorNameFromParam(const std::string & param_name,
                                             const bool allow_invalid) const
{
  const auto valid_param = isParamValid(param_name);
  if (!allow_invalid)
  {
    if (!valid_param)
      mooseError("Failed to get a parameter with the name \"",
                 param_name,
                 "\" when getting a MeshGenerator.",
                 "\n\nKnown parameters:\n",
                 _pars);
    if (!_pars.isType<MeshGeneratorName>(param_name))
      paramError(param_name,
                 "Parameter of type \"",
                 _pars.type(param_name),
                 "\" is not an expected type for getting a MeshGenerator (should be of type "
                 "\"MeshGeneratorName\")");
  }
  else if (!valid_param)
    return nullptr;

  const auto & name = getParam<MeshGeneratorName>(param_name);
  checkGetMesh(name, param_name);

  return &name;
}

const std::vector<MeshGeneratorName> &
MeshGenerator::getMeshGeneratorNamesFromParam(const std::string & param_name) const
{
  if (!isParamValid(param_name))
    mooseError("Failed to get a parameter with the name \"",
               param_name,
               "\" when getting MeshGenerators.",
               "\n\nKnown parameters:\n",
               _pars);
  if (!_pars.isType<std::vector<MeshGeneratorName>>(param_name))
    paramError(param_name,
               "Parameter of type \"",
               _pars.type(param_name),
               "\" is not an expected type for getting MeshGenerators (should be of type "
               "\"std::vector<MeshGeneratorName>\")");

  const auto & names = getParam<std::vector<MeshGeneratorName>>(param_name);
  for (const auto & name : names)
    checkGetMesh(name, param_name);

  return names;
}

void
MeshGenerator::checkGetMesh(const MeshGeneratorName & mesh_generator_name,
                            const std::string & param_name) const
{
  mooseAssert(!mesh_generator_name.empty(), "Empty name");
  const auto & mg_sys = _app.getMeshGeneratorSystem();
  if (!_app.constructingMeshGenerators())
    mooseError("Cannot get a mesh outside of construction");
  if (!mg_sys.hasMeshGenerator(mesh_generator_name) && !isNullMeshName(mesh_generator_name))
  {
    std::stringstream error;
    error << "The requested MeshGenerator with name '" << mesh_generator_name << "' ";
    if (mg_sys.hasMeshGeneratorParams(mesh_generator_name))
      error << "was found, but has not been constructed yet.\n\nThis can occur when your "
               "dependencies are not properly defined and we cannot infer the proper construction "
               "order of your MeshGenerators.\n\nThe most likely case is a sub generator whose "
               "input(s) are not declared as a sub dependency in the generator creating them.";
    else
      error << "was not found.";
    if (param_name.size())
      paramError(param_name, error.str());
    else
      mooseError(error.str());
  }
}

std::unique_ptr<MeshBase> &
MeshGenerator::getMesh(const std::string & param_name, const bool allow_invalid /* = false */)
{
  const MeshGeneratorName * name = getMeshGeneratorNameFromParam(param_name, allow_invalid);
  if (!name)
    return _null_mesh;
  return getMeshByName(*name);
}

std::vector<std::unique_ptr<MeshBase> *>
MeshGenerator::getMeshes(const std::string & param_name)
{
  return getMeshesByName(getMeshGeneratorNamesFromParam(param_name));
}

std::unique_ptr<MeshBase> &
MeshGenerator::getMeshByName(const MeshGeneratorName & mesh_generator_name)
{
  checkGetMesh(mesh_generator_name, "");
  if (isNullMeshName(mesh_generator_name))
    return _null_mesh;

  _requested_mesh_generators.insert(mesh_generator_name);
  auto & mesh = _app.getMeshGeneratorSystem().getMeshGeneratorOutput(mesh_generator_name);
  _requested_meshes.emplace_back(mesh_generator_name, &mesh);
  return mesh;
}

std::vector<std::unique_ptr<MeshBase> *>
MeshGenerator::getMeshesByName(const std::vector<MeshGeneratorName> & mesh_generator_names)
{
  std::vector<std::unique_ptr<MeshBase> *> meshes;
  for (const auto & name : mesh_generator_names)
    meshes.push_back(&getMeshByName(name));
  return meshes;
}

void
MeshGenerator::declareMeshForSub(const std::string & param_name)
{
  declareMeshForSubByName(*getMeshGeneratorNameFromParam(param_name, false));
}

void
MeshGenerator::declareMeshesForSub(const std::string & param_name)
{
  declareMeshesForSubByName(getMeshGeneratorNamesFromParam(param_name));
}

void
MeshGenerator::declareMeshForSubByName(const MeshGeneratorName & mesh_generator_name)
{
  checkGetMesh(mesh_generator_name, "");
  if (isNullMeshName(mesh_generator_name))
    return;

  _requested_mesh_generators_for_sub.insert(mesh_generator_name);
}

void
MeshGenerator::declareMeshesForSubByName(
    const std::vector<MeshGeneratorName> & mesh_generator_names)
{
  for (const auto & name : mesh_generator_names)
    declareMeshForSubByName(name);
}

std::unique_ptr<MeshBase>
MeshGenerator::buildMeshBaseObject(unsigned int dim)
{
  mooseAssert(_mesh, "Need a MooseMesh object");
  return _mesh->buildMeshBaseObject(dim);
}

std::unique_ptr<ReplicatedMesh>
MeshGenerator::buildReplicatedMesh(unsigned int dim)
{
  mooseAssert(_mesh, "Need a MooseMesh object");
  return _mesh->buildTypedMesh<ReplicatedMesh>(dim);
}

std::unique_ptr<DistributedMesh>
MeshGenerator::buildDistributedMesh(unsigned int dim)
{
  mooseAssert(_mesh, "Need a MooseMesh object");
  return _mesh->buildTypedMesh<DistributedMesh>(dim);
}

std::unique_ptr<MeshBase>
MeshGenerator::generateInternal()
{
  libmesh_parallel_only(comm());
  mooseAssert(comm().verify(type() + name()), "Inconsistent execution ordering");

#ifndef NDEBUG
  for (const auto & [name, mesh] : _requested_meshes)
    mooseAssert(*mesh, "Null output from " + name);
#endif

  auto mesh = generate();
  mooseAssert(mesh, "Null output");

  for (const auto & [requested_name, requested_mesh] : _requested_meshes)
    if (*requested_mesh)
      mooseError(
          "The mesh from input ",
          _app.getMeshGenerator(requested_name).type(),
          " '",
          _app.getMeshGenerator(requested_name).name(),
          "' was not moved.\n\nThe MeshGenerator system requires that the memory from all input "
          "meshes\nare managed by the requesting MeshGenerator during the generate phase.\n\nThis "
          "is achieved with a std::move() operation within the generate() method.");

  if (getParam<bool>("show_info"))
  {
    const auto mesh_info = mesh->get_info(/* verbosity = */ 2);

    // We will prefix all information with "type() 'name()':" because this could potentially
    // output a ton of information and looks a bit better with a prefix
    std::stringstream oss;
    const auto split = MooseUtils::split(mesh_info, "\n");
    if (split.size())
      for (std::size_t i = 0; i < split.size() - 1; ++i) // ignore the last line break
        oss << COLOR_CYAN << "" << type() << " '" << name() << "': " << COLOR_DEFAULT << split[i]
            << std::endl;
    _console << oss.str() << std::flush;
  }

  // output the current mesh block to file
  if (getParam<bool>("output"))
  {
    if (!getParam<bool>("nemesis"))
    {
      ExodusII_IO exio(*mesh);

      if (mesh->mesh_dimension() == 1)
        exio.write_as_dimension(3);

      // Default to non-HDF5 output for wider compatibility
      exio.set_hdf5_writing(false);

      exio.write(name() + "_in.e");
    }
    else
    {
      Nemesis_IO nemesis_io(*mesh);

      // Default to non-HDF5 output for wider compatibility
      nemesis_io.set_hdf5_writing(false);

      nemesis_io.write(name() + "_in.e");
    }
  }

  return mesh;
}

void
MeshGenerator::addMeshSubgenerator(const std::string & type,
                                   const std::string & name,
                                   InputParameters params)
{
  if (!_app.constructingMeshGenerators())
    mooseError("Can only call addMeshSubgenerator() during MeshGenerator construction");

  // In case the user forgot it
  params.set<MooseApp *>("_moose_app") = &_app;

  _app.addMeshGenerator(type, name, params);
  _sub_mesh_generators.insert(&std::as_const(_app).getMeshGenerator(name));
}

RestartableDataValue &
MeshGenerator::setMeshPropertyHelper(const std::string & data_name)
{
  return _app.getRestartableMetaData(meshPropertyName(data_name), MooseApp::MESH_META_DATA, 0);
}

void
MeshGenerator::addParentMeshGenerator(const MeshGenerator & mg, const AddParentChildKey)
{
  mooseAssert(_app.constructingMeshGenerators(), "Should only be called at construction");
  _parent_mesh_generators.insert(&mg);
}

void
MeshGenerator::addChildMeshGenerator(const MeshGenerator & mg, const AddParentChildKey)
{
  mooseAssert(_app.constructingMeshGenerators(), "Should only be called at construction");
  _child_mesh_generators.insert(&mg);
}

bool
MeshGenerator::isParentMeshGenerator(const MeshGeneratorName & name,
                                     const bool direct /* = true */) const
{
  return std::find_if(getParentMeshGenerators().begin(),
                      getParentMeshGenerators().end(),
                      [&name, &direct](const auto & mg)
                      {
                        return mg->name() == name ||
                               (!direct && mg->isParentMeshGenerator(name, /* direct = */ false));
                      }) != getParentMeshGenerators().end();
}

bool
MeshGenerator::isChildMeshGenerator(const MeshGeneratorName & name,
                                    const bool direct /* = true */) const
{
  return std::find_if(getChildMeshGenerators().begin(),
                      getChildMeshGenerators().end(),
                      [&name, &direct](const auto & mg)
                      {
                        return mg->name() == name ||
                               (!direct && mg->isChildMeshGenerator(name, /* direct = */ false));
                      }) != getChildMeshGenerators().end();
}

void
MeshGenerator::declareNullMeshName(const MeshGeneratorName & name)
{
  mooseAssert(_app.constructingMeshGenerators(), "Should only be called at construction");
  mooseAssert(!_null_mesh_names.count(name), "Already declared");
  _null_mesh_names.insert(name);
}

bool
MeshGenerator::hasSaveMesh()
{
  return _save_with_name.size();
}

const std::string &
MeshGenerator::getSavedMeshName() const
{
  return _save_with_name;
}
