//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SplitMeshAction.h"

#include "MooseApp.h"
#include "MooseUtils.h"
#include "MooseMesh.h"

#include <filesystem>

#include "libmesh/checkpoint_io.h"

registerMooseAction("MooseApp", SplitMeshAction, "split_mesh");

InputParameters
SplitMeshAction::validParams()
{
  return Action::validParams();
}

SplitMeshAction::SplitMeshAction(const InputParameters & params) : Action(params) {}

void
SplitMeshAction::act()
{
  auto mesh = _app.actionWarehouse().mesh();
  const std::string split_file_arg = _app.parameters().isParamSetByUser("split_file")
                                         ? _app.parameters().get<std::string>("split_file")
                                         : "";

  if (mesh->getFileName() == "" && split_file_arg == "")
    mooseError("Output mesh file name must be specified (with --split-file) when splitting "
               "non-file-based meshes");

  auto splitstr = _app.parameters().get<std::string>("split_mesh");
  std::vector<unsigned int> splits;
  bool success = MooseUtils::tokenizeAndConvert(splitstr, splits, ", ");
  if (!success)
    mooseError("invalid argument for --split-mesh: '", splitstr, "'");

  // Decide whether to create ASCII or binary splits based on the split_file_arg. We use the
  // following rules to decide:
  // 1.) No file extension -> ASCII + gzip
  // 2.) .cpr file extension -> binary
  // 3.) .cpa.gz file extension -> ASCII + gzip
  // 4.) Any other file extension -> mooseError

  // Get the file extension without the dot.
  std::string split_file_arg_ext = MooseUtils::getExtension(split_file_arg);

  bool checkpoint_binary_flag = false;

  if (split_file_arg_ext != "")
  {
    if (split_file_arg_ext == "cpr")
      checkpoint_binary_flag = true;
    else if (split_file_arg_ext == "cpa.gz")
      checkpoint_binary_flag = false;
    else
      mooseError("The argument to --split-file, ",
                 split_file_arg,
                 ", must not end in a file extension other than .cpr or .cpa.gz");
  }

  // To name the split files, we start with the given mesh filename
  // (if set) or the argument to --split-file, strip any existing
  // extension, and then append either .cpr or .cpa.gz depending on the
  // checkpoint_binary_flag.
  auto fname = mesh->getFileName();
  if (fname == "")
    fname = split_file_arg;
  fname = MooseUtils::stripExtension(fname) + (checkpoint_binary_flag ? ".cpr" : ".cpa.gz");

  for (std::size_t i = 0; i < splits.size(); i++)
  {
    processor_id_type n = splits[i];
    Moose::out << "Splitting " << n << " ways..." << std::endl;

    auto cp = libMesh::split_mesh(*mesh, n);
    Moose::out << "    - writing " << cp->current_processor_ids().size() << " files per process..."
               << std::endl;
    cp->binary() = checkpoint_binary_flag;

    // different splits will be written into subfolders with n being the folder name
    cp->write(fname);
  }

  // Write mesh metadata
  if (processor_id() == 0)
  {
    const auto filenames = _app.writeRestartableMetaData(MooseApp::MESH_META_DATA, fname);
    Moose::out << "Mesh meta data written into "
               << std::filesystem::absolute(filenames[0].parent_path()) << "." << std::endl;
  }
}
