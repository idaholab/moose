//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EBSDMeshGenerator.h"
#include "MooseApp.h"
#include "MathUtils.h"

#include "libmesh/int_range.h"
#include "libmesh/mesh_refinement.h"

#include <fstream>

registerMooseObject("PhaseFieldApp", EBSDMeshGenerator);

InputParameters
EBSDMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("Mesh generated from a specified DREAM.3D EBSD data file.");
  params.addRequiredParam<FileName>("filename", "The name of the file containing the EBSD data");

  params.addDeprecatedParam<unsigned int>("uniform_refine",
                                          "Deprecated. Use `pre_refine` instead.",
                                          "Deprecated. Use `pre_refine` instead.");
  params.addParam<unsigned int>(
      "pre_refine",
      0,
      "Number of coarsening levels available in adaptive mesh refinement. The resulting mesh will "
      "have one mesh element per EBSD data cell, but will be based on a refined coarser mesh with "
      "'pre_refine' levels of refinement. This requires all dimension of the EBSD data to be "
      "divisible by 2^pre_refine.(this parameter was formerly called 'uniform_refine')");

  params.addParam<processor_id_type>(
      "num_cores_for_partition",
      "Number of cores for partitioning the graph (dafaults to the number of MPI ranks)");
  return params;
}

EBSDMeshGenerator::EBSDMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _distributed(_mesh->isDistributedMesh()),
    _filename(getParam<FileName>("filename")),
    _pre_refine(isParamValid("pre_refine") ? getParam<unsigned int>("pre_refine")
                                           : getParam<unsigned int>("uniform_refine")),
    _base(buildMeshSubgenerator())
{
}

std::unique_ptr<MeshBase> &
EBSDMeshGenerator::buildMeshSubgenerator()
{
  readEBSDHeader();

  const auto generator_type =
      _distributed ? "DistributedRectilinearMeshGenerator" : "GeneratedMeshGenerator";
  auto params = _app.getFactory().getValidParams(generator_type);

  params.set<MooseEnum>("dim") = (_geometry.dim == 1 ? "1" : (_geometry.dim == 2 ? "2" : "3"));

  std::array<unsigned int, 3> nr;
  nr[0] = _geometry.n[0];
  nr[1] = _geometry.n[1];
  nr[2] = _geometry.n[2];

  // set min/max box length
  params.set<Real>("xmin") = _geometry.min[0];
  params.set<Real>("xmax") = nr[0] * _geometry.d[0] + _geometry.min[0];
  params.set<Real>("ymin") = _geometry.min[1];
  params.set<Real>("ymax") = nr[1] * _geometry.d[1] + _geometry.min[1];
  params.set<Real>("zmin") = _geometry.min[2];
  params.set<Real>("zmax") = nr[2] * _geometry.d[2] + _geometry.min[2];

  // check if the requested uniform refine level is possible and determine initial grid size
  for (auto i : make_range(_geometry.dim))
  {
    auto factor = MathUtils::pow(2, _pre_refine);
    if (nr[i] % factor != 0)
      paramError("pre_refine",
                 "EBSDMeshGenerator error. Requested levels of pre refinement not possible.");
    nr[i] /= factor;
  }

  if (_distributed)
  {
    params.set<dof_id_type>("nx") = nr[0];
    params.set<dof_id_type>("ny") = nr[1];
    params.set<dof_id_type>("nz") = nr[2];
    params.set<processor_id_type>("num_cores_for_partition") =
        isParamValid("num_cores_for_partition")
            ? getParam<processor_id_type>("num_cores_for_partition")
            : 0;
  }
  else
  {
    params.set<unsigned int>("nx") = nr[0];
    params.set<unsigned int>("ny") = nr[1];
    params.set<unsigned int>("nz") = nr[2];
  }

  addMeshSubgenerator(generator_type, name() + "_base_mesh", params);
  return getMeshByName(name() + "_base_mesh");
}

void
EBSDMeshGenerator::readEBSDHeader()
{
  std::ifstream stream_in(_filename.c_str());

  if (!stream_in)
    paramError("filename", "Can't open EBSD file: ", _filename);

  // Labels to look for in the header
  const std::vector<std::string> labels = {
      "x_step", "x_dim", "y_step", "y_dim", "z_step", "z_dim", "x_min", "y_min", "z_min"};

  // Dimension variables to store once they are found in the header
  // X_step, X_Dim, Y_step, Y_Dim, Z_step, Z_Dim
  // We use Reals even though the Dim values should all be integers...
  std::vector<Real> label_vals(labels.size(), 0.0);

  std::string line;
  while (std::getline(stream_in, line))
  {
    // We need to process the comment lines that have:
    // X_step, X_Dim
    // Y_step, Y_Dim
    // Z_step, Z_Dim
    // in them. The labels are case insensitive.
    if (line.find("#") == 0)
    {
      // Process lines that start with a comment character (comments and meta data)
      std::transform(line.begin(), line.end(), line.begin(), ::tolower);

      for (unsigned i = 0; i < labels.size(); ++i)
        if (line.find(labels[i]) != std::string::npos)
        {
          std::string dummy;
          std::istringstream iss(line);
          iss >> dummy >> dummy >> label_vals[i];

          // One label per line, break out of for loop over labels
          break;
        }
    }
    else
      // first non comment line marks the end of the header
      break;
  }

  // Copy stuff out of the label_vars array into class variables
  _geometry.d[0] = label_vals[0];
  _geometry.n[0] = label_vals[1];
  _geometry.min[0] = label_vals[6];

  _geometry.d[1] = label_vals[2];
  _geometry.n[1] = label_vals[3];
  _geometry.min[1] = label_vals[7];

  _geometry.d[2] = label_vals[4];
  _geometry.n[2] = label_vals[5];
  _geometry.min[2] = label_vals[8];

  unsigned int dim;

  // determine mesh dimension
  for (dim = 3; dim > 0 && _geometry.n[dim - 1] == 0; --dim)
    ;

  // check if the data has nonzero stepsizes
  for (unsigned i = 0; i < dim; ++i)
  {
    if (_geometry.n[i] == 0)
      mooseError("Error reading header, EBSD grid size is zero.");
    if (_geometry.d[i] == 0.0)
      mooseError("Error reading header, EBSD data step size is zero.");
  }

  if (dim == 0)
    mooseError("Error reading header, EBSD data is zero dimensional.");

  _geometry.dim = dim;
}

std::unique_ptr<MeshBase>
EBSDMeshGenerator::generate()
{
  if (_pre_refine)
  {
    MeshRefinement mesh_refinement(*_base);
    mesh_refinement.uniformly_refine(_pre_refine);
  }
  return std::move(_base);
}
