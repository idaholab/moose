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

defineLegacyParams(MeshGenerator);

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

std::unique_ptr<MeshBase> &
MeshGenerator::getMesh(const std::string & input_mesh_generator_parameter_name)
{
  if (isParamValid(input_mesh_generator_parameter_name))
  {
    auto name = getParam<MeshGeneratorName>(input_mesh_generator_parameter_name);

    _depends_on.push_back(name);

    return _app.getMeshGeneratorOutput(name);
  }
  else
    return _null_mesh;
}

std::unique_ptr<MeshBase> &
MeshGenerator::getMeshByName(const MeshGeneratorName & input_mesh_generator)
{
  _depends_on.push_back(input_mesh_generator);
  return _app.getMeshGeneratorOutput(input_mesh_generator);
}

std::unique_ptr<MeshBase>
MeshGenerator::buildMeshBaseObject(unsigned int dim)
{
  if (!_mesh)
    mooseError("We need a MooseMesh object in order to build ReplicatedMesh objects through the "
               "buildReplicatedMesh API");
  return _mesh->buildMeshBaseObject(dim);
}

std::unique_ptr<ReplicatedMesh>
MeshGenerator::buildReplicatedMesh(unsigned int dim)
{
  if (!_mesh)
    mooseError("We need a MooseMesh object in order to build ReplicatedMesh objects through the "
               "buildReplicatedMesh API");
  return _mesh->buildTypedMesh<ReplicatedMesh>(dim);
}

std::unique_ptr<DistributedMesh>
MeshGenerator::buildDistributedMesh(unsigned int dim)
{
  if (!_mesh)
    mooseError("We need a MooseMesh object in order to build DistributedMesh objects through the "
               "buildDistributedMesh API");
  return _mesh->buildTypedMesh<DistributedMesh>(dim);
}

std::unique_ptr<MeshBase>
MeshGenerator::generateInternal()
{
  auto mesh = generate();

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
MeshGenerator::addMeshSubgenerator(const std::string & generator_name,
                                   const std::string & name,
                                   InputParameters & params)
{
  // In case the user forgot it
  params.set<MooseApp *>("_moose_app") = &_app;

  _app.addMeshGenerator(generator_name, name, params);

  return this->getMeshByName(name);
}
