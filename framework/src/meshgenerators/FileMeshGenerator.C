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

#include "libmesh/replicated_mesh.h"
#include "libmesh/face_quad4.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/sparse_matrix.h"

registerMooseObject("MooseApp", FileMeshGenerator);

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
  params.addParam<bool>("skip_partitioning",
                        false,
                        "True to skip partitioning, only after this mesh generator, "
                        "because the mesh was pre-split for example.");
  params.addParam<bool>("allow_renumbering",
                        true,
                        "Whether to allow the mesh to renumber nodes and elements, if not "
                        "overridden by a global parameter or by a requirement (e.g. an exodus "
                        "restart or a constraint matrix) that disables renumbering.");
  params.addParam<bool>("clear_spline_nodes",
                        false,
                        "If clear_spline_nodes=true, IsoGeometric Analyis spline nodes "
                        "and constraints are removed from an IGA mesh, after which only "
                        "C^0 Rational-Bernstein-Bezier elements will remain.");
  params.addParam<bool>("discontinuous_spline_extraction",
                        false,
                        "If discontinuous_spline_extraction=true, "
                        "Rational-Bernstein-Bezier elements extracted from a spline mesh "
                        "will be disconnected from neighboring elements, coupled only via "
                        "their extraction operators.  This may be less efficient than the "
                        "default C^0 extracted mesh, but may be necessary if the extracted "
                        "mesh is non-conforming.");
  params.addParam<MatrixFileName>(
      "constraint_matrix", "", "The name of a constraint matrix file to apply to the mesh");
  params.addClassDescription("Read a mesh from a file.");
  params.addParamNamesToGroup(
      "clear_spline_nodes discontinuous_spline_extraction constraint_matrix",
      "IsoGeometric Analysis (IGA) and other mesh constraint options");
  return params;
}

FileMeshGenerator::FileMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _file_name(getParam<MeshFileName>("file")),
    _matrix_file_name(getParam<MatrixFileName>("constraint_matrix")),
    _skip_partitioning(getParam<bool>("skip_partitioning")),
    _allow_renumbering(getParam<bool>("allow_renumbering"))
{
}

std::unique_ptr<MeshBase>
FileMeshGenerator::generate()
{
  auto mesh = buildMeshBaseObject();

  // Maybe we'll reallow renumbering after constraints are applied?
  bool eventually_allow_renumbering = _allow_renumbering && mesh->allow_renumbering();

  // If we have a constraint matrix, we need its numbering to match
  // the numbering in the mesh file
  if (!_matrix_file_name.empty())
    mesh->allow_renumbering(false);

  // Figure out if we are reading an Exodus file, but not Tetgen (*.ele)
  bool exodus = (_file_name.rfind(".exd") < _file_name.size() ||
                 _file_name.rfind(".e") < _file_name.size()) &&
                _file_name.rfind(".ele") == std::string::npos;
  bool has_exodus_integers = isParamValid("exodus_extra_element_integers");
  bool restart_exodus = (getParam<bool>("use_for_exodus_restart") && _app.getExodusFileRestart());
  if (exodus)
  {
    auto exreader = std::make_shared<libMesh::ExodusII_IO>(*mesh);
    MooseUtils::checkFileReadable(_file_name);

    if (has_exodus_integers)
      exreader->set_extra_integer_vars(
          getParam<std::vector<std::string>>("exodus_extra_element_integers"));

    if (restart_exodus)
    {
      _app.setExReaderForRestart(std::move(exreader));
      exreader->read(_file_name);
      eventually_allow_renumbering = false;
      mesh->allow_renumbering(false);
    }
    else
    {
      if (mesh->processor_id() == 0)
      {
        if (getParam<bool>("discontinuous_spline_extraction"))
          exreader->set_discontinuous_bex(true);

        exreader->read(_file_name);

        if (getParam<bool>("clear_spline_nodes"))
          MeshTools::clear_spline_nodes(*mesh);
      }
      libMesh::MeshCommunication().broadcast(*mesh);
    }
    // Skip partitioning if the user requested it
    if (_skip_partitioning)
      mesh->skip_partitioning(true);
    mesh->prepare_for_use();
    mesh->skip_partitioning(false);
  }
  else
  {
    if (_pars.isParamSetByUser("exodus_extra_element_integers"))
      mooseError("\"exodus_extra_element_integers\" should be given only for Exodus mesh files");
    if (_pars.isParamSetByUser("use_for_exodus_restart"))
      mooseError("\"use_for_exodus_restart\" should be given only for Exodus mesh files");

    const auto file_name = deduceCheckpointPath(*this, _file_name);
    MooseUtils::checkFileReadable(file_name);

    mesh->skip_partitioning(_skip_partitioning);
    mesh->allow_renumbering(_allow_renumbering);
    mesh->read(file_name);

    // Load the meta data if it is available
    _app.possiblyLoadRestartableMetaData(MooseApp::MESH_META_DATA, (std::string)file_name);
  }

  if (!_matrix_file_name.empty())
  {
    auto matrix = SparseMatrix<Number>::build(mesh->comm());
    matrix->read_matlab(_matrix_file_name);

    // In the future we might deduce matrix orientation via matrix
    // size; for now we simply hardcode that the Flex IGA standard for
    // projection operator matrices is the transpose of our standard
    // for constraint equations.
    matrix->get_transpose(*matrix);
    mesh->copy_constraint_rows(*matrix);

    // libMesh should probably update this in copy_constraint_rows();
    // once it does this will be a redundant sweep we can remove.
    mesh->cache_elem_data();
  }

  mesh->allow_renumbering(eventually_allow_renumbering);

  return dynamic_pointer_cast<MeshBase>(mesh);
}

std::string
FileMeshGenerator::deduceCheckpointPath(const MooseObject & object, const std::string & file_name)
{
  // Just exists, use it
  if (MooseUtils::pathExists(file_name))
    return file_name;

  // LATEST
  return MooseUtils::convertLatestCheckpoint(file_name) + object.getMooseApp().checkpointSuffix();
}
