//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmoothMeshGenerator.h"

// libMesh includes
#include "libmesh/mesh_smoother_laplace.h"
#include "libmesh/mesh_smoother_vsmoother.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/replicated_mesh.h"

#include "CastUniquePointer.h"

registerMooseObject("MooseApp", SmoothMeshGenerator);

InputParameters
SmoothMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to smooth.");
  params.addClassDescription(
      "Utilizes the specified smoothing algorithm to attempt to improve "
      "mesh quality.");

  MooseEnum SmootherAlgorithm("variational laplace", "variational");
  params.addParam<MooseEnum>("algorithm", SmootherAlgorithm, "The smoothing algorithm to use.");
  params.addParam<unsigned int>(
      "iterations", 1, "Laplace algorithm only: the number of smoothing iterations to do.");
  params.addRangeCheckedParam<Real>(
      "dilation_weight",
      0.5,
      "dilation_weight >= 0.0 & dilation_weight <= 1.0",
      "Variational algorithm only: the weight of the dilation metric. The distortion metric is "
      "given weight 1 - dilation_weight.");
  params.addParam<bool>("preserve_subdomain_boundaries",
                        true,
                        "Variational algorithm only: whether the input mesh's subdomain boundaries "
                        "should be preserved during the smoothing process.");
  params.addParam<Real>("relative_residual_tolerance",
                        TOLERANCE * TOLERANCE,
                        "Variational algorithm only: solver relative residual tolerance.");
  params.addParam<Real>("absolute_residual_tolerance",
                        TOLERANCE * TOLERANCE,
                        "Variational algorithm only: solver absolute residual tolerance.");
  params.addRangeCheckedParam<unsigned int>(
      "verbosity",
      1,
      "0 <= verbosity <= 100",
      "Variational algorithm only: verbosity level between 0 and 100.");

  return params;
}

SmoothMeshGenerator::SmoothMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _algorithm(getParam<MooseEnum>("algorithm")),
    _iterations(getParam<unsigned int>("iterations")),
    _dilation_weight(getParam<Real>("dilation_weight")),
    _preserve_subdomain_boundaries(getParam<bool>("preserve_subdomain_boundaries")),
    _relative_residual_tolerance(getParam<Real>("relative_residual_tolerance")),
    _absolute_residual_tolerance(getParam<Real>("absolute_residual_tolerance")),
    _verbosity(getParam<unsigned int>("verbosity"))
{
  if (isParamSetByUser("algorithm"))
  {
    // Warn the user if they try to mix laplace smoother params with variational
    // smoother params

    std::vector<std::string> check_params;
    std::string other_algorithm;
    if (_algorithm == "laplace")
    {
      other_algorithm = "variational";
      check_params = {"dilation_weight",
                      "preserve_subdomain_boundaries",
                      "relative_residual_tolerance",
                      "absolute_residual_tolerance",
                      "verbosity"};
    }

    else // _algorithm == "variational"
    {
      other_algorithm = "laplace";
      check_params = {"iterations"};
    }

    for (const auto & param_name : check_params)
      if (isParamSetByUser(param_name))
        mooseError(
                     " param '",
                     param_name,
                     "' applies to algorithm='",
                     other_algorithm,
                     "' only and has no effect on the ",
                     "currently selected algorithm='",
                     _algorithm,
                     "'.");
  }
}

std::unique_ptr<MeshBase>
SmoothMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = nullptr;
  std::unique_ptr<libMesh::MeshSmoother> smoother = nullptr;

  if (_algorithm == "laplace")
  {
    std::unique_ptr<MeshBase> old_mesh = std::move(_input);
    if (!old_mesh->is_replicated())
      mooseError(
          "SmoothMeshGenerator with algorithm='laplace' is not implemented for distributed meshes");

    mesh = dynamic_pointer_cast<ReplicatedMesh>(old_mesh);

    smoother = std::make_unique<libMesh::LaplaceMeshSmoother>(
        static_cast<UnstructuredMesh &>(*mesh), _iterations);
  }

  else if (_algorithm == "variational")
  {
    mesh = std::move(_input);
    smoother =
        std::make_unique<libMesh::VariationalMeshSmoother>(static_cast<UnstructuredMesh &>(*mesh),
                                                           _dilation_weight,
                                                           _preserve_subdomain_boundaries,
                                                           _relative_residual_tolerance,
                                                           _absolute_residual_tolerance,
                                                           _verbosity);
  }

  smoother->smooth();

  return dynamic_pointer_cast<MeshBase>(mesh);
}
