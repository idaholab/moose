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

InputParameters
MeshGenerator::validParams()
{
  InputParameters params = MooseObject::validParams();

  params.addParam<bool>("show_info",
                        false,
                        "Whether or not to show mesh info after generating the mesh "
                        "(bounding box, element types, sidesets, nodesets, subdomains, etc)");

  params.registerBase("MeshGenerator");

  return params;
}

MeshGenerator::MeshGenerator(const InputParameters & parameters)
  : MooseObject(parameters), MeshMetaDataInterface(this), _mesh(_app.actionWarehouse().mesh())
{
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
  if (!std::as_const(_app).hasMeshGenerator(name))
    paramError(param_name, "Requested MeshGenerator with name '", name, "' was not found");

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
    if (!std::as_const(_app).hasMeshGenerator(name))
      paramError(param_name, "The requested MeshGenerator '", name, "' was not found");

  return names;
}

void
MeshGenerator::checkGetMesh(const MeshGeneratorName & mesh_generator_name) const
{
  mooseAssert(!mesh_generator_name.empty(), "Empty name");
  if (!_app.constructingMeshGenerators())
    mooseError("Cannot get a mesh outside of construction");
  if (!std::as_const(_app).hasMeshGenerator(mesh_generator_name))
    mooseError("The requested MeshGenerator '", mesh_generator_name, "' was not found");
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
  checkGetMesh(mesh_generator_name);
  _requested_mesh_generators.insert(mesh_generator_name);
  auto & mesh = _app.getMeshGeneratorOutput(mesh_generator_name);
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
MeshGenerator::getMeshForSub(const std::string & param_name)
{
  getMeshForSubByName(*getMeshGeneratorNameFromParam(param_name, false));
}

void
MeshGenerator::getMeshesForSub(const std::string & param_name)
{
  getMeshesForSubByName(getMeshGeneratorNamesFromParam(param_name));
}

void
MeshGenerator::getMeshForSubByName(const MeshGeneratorName & mesh_generator_name)
{
  checkGetMesh(mesh_generator_name);
  _requested_mesh_generators_for_sub.insert(mesh_generator_name);
}

void
MeshGenerator::getMeshesForSubByName(const std::vector<MeshGeneratorName> & mesh_generator_names)
{
  for (const auto & name : mesh_generator_names)
    getMeshForSubByName(name);
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

  auto mesh = generate();
  mooseAssert(mesh, "Invalid mesh");

  for (const auto & [requested_name, requested_mesh] : _requested_meshes)
    if (*requested_mesh)
      mooseError("The mesh from input ",
                 std::as_const(_app).getMeshGenerator(requested_name).type(),
                 " '",
                 std::as_const(_app).getMeshGenerator(requested_name).name(),
                 "' was not taken ownership of.");

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

  return mesh;
}

std::unique_ptr<MeshBase> &
MeshGenerator::addMeshSubgenerator(const std::string & type,
                                   const std::string & name,
                                   InputParameters & params)
{
  if (!_app.constructingMeshGenerators())
    mooseError("Can only call addMeshSubgenerator() during MeshGenerator construction");

  // In case the user forgot it
  params.set<MooseApp *>("_moose_app") = &_app;

  _app.addMeshGenerator(type, name, params);
  _sub_mesh_generators.insert(&std::as_const(_app).getMeshGenerator(name));
  return this->getMeshByName(name);
}

RestartableDataValue &
MeshGenerator::setMeshPropertyHelper(const std::string & data_name)
{
  return _app.getRestartableMetaData(meshPropertyName(data_name), MooseApp::MESH_META_DATA, 0);
}

void
MeshGenerator::addParentMeshGenerator(const MeshGenerator & mg, const AddParentChildKey)
{
  _parent_mesh_generators.insert(&mg);
}

void
MeshGenerator::addChildMeshGenerator(const MeshGenerator & mg, const AddParentChildKey)
{
  _child_mesh_generators.insert(&mg);
}

bool
MeshGenerator::isParentMeshGenerator(const MeshGeneratorName & name) const
{
  return std::find_if(getParentMeshGenerators().begin(),
                      getParentMeshGenerators().end(),
                      [&name](const auto & mg)
                      { return mg->name() == name; }) != getParentMeshGenerators().end();
}
