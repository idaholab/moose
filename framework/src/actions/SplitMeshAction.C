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
#include "SerializerGuard.h"

#include "libmesh/checkpoint_io.h"

registerMooseAction("MooseApp", SplitMeshAction, "split_mesh");

template <>
InputParameters
validParams<SplitMeshAction>()
{
  return validParams<Action>();
}

SplitMeshAction::SplitMeshAction(InputParameters params) : Action(params) {}

void
SplitMeshAction::act()
{
  auto mesh = _app.actionWarehouse().mesh();
  auto split_file_arg = _app.parameters().get<std::string>("split_file");

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
  // 1.) No file extension -> binary
  // 2.) .cpr file extension -> binary
  // 3.) .cpa file extension -> ASCII
  // 4.) Any other file extension -> mooseError

  // Get the file extension without the dot.
  // TODO: Maybe this should be in MooseUtils?
  std::string split_file_arg_ext;
  auto pos = split_file_arg.rfind(".");
  if (pos != std::string::npos)
    split_file_arg_ext = split_file_arg.substr(pos + 1, std::string::npos);

  // If stripExtension() returns the original string, then there is no
  // file extension or the original string was empty.
  bool checkpoint_binary_flag = true;

  if (split_file_arg_ext != "")
  {
    if (split_file_arg_ext == "cpr")
      checkpoint_binary_flag = true;
    else if (split_file_arg_ext == "cpa")
      checkpoint_binary_flag = false;
    else
      mooseError("The argument to --split-file, ",
                 split_file_arg,
                 ", must not end in a file extension other than .cpr or .cpa");
  }

  for (std::size_t i = 0; i < splits.size(); i++)
  {
    processor_id_type n = splits[i];
    Moose::out << "Splitting " << n << " ways..." << std::endl;

    auto cp = libMesh::split_mesh(*mesh, n);

    auto fname = mesh->getFileName();

    // To name the split files, we start with the given mesh filename
    // (if set) or the argument to --split-file, strip any existing
    // extension, and then append either .cpr or .cpa depending on the
    // checkpoint_binary_flag.
    if (fname == "" || split_file_arg != "")
      fname = split_file_arg;
    fname = MooseUtils::stripExtension(fname) + (checkpoint_binary_flag ? ".cpr" : ".cpa");

    Moose::out << "    - writing " << cp->current_processor_ids().size() << " files per process\n"
               << " to " << fname << std::endl;

    // Check the file directory of file_base and create if needed
    std::string base = fname;
    base = base.substr(0, base.find_last_of('/'));

    {
      // Serialize this in case all procs are writing to the same place (they might not be - which
      // is why we need to do this!
      SerializerGuard guard(mesh->comm(), false);

      if (access(base.c_str(), W_OK) == -1)
      {
        // Directory does not exist. Loop through incremental directories and create as needed.
        std::vector<std::string> path_names;
        MooseUtils::tokenize(base, path_names);
        std::string inc_path;

        // If it's an absolute path keep it that way!
        if (base[0] == '/')
          inc_path = '/';

        inc_path = inc_path + path_names[0];

        for (unsigned int i = 1; i < path_names.size(); ++i)
        {
          inc_path += '/' + path_names[i];
          if (access(inc_path.c_str(), W_OK) == -1)
            if (mkdir(inc_path.c_str(), S_IRWXU | S_IRGRP) == -1)
              mooseError("Could not create directory: " + inc_path + " for: " + fname);
        }
      }
    }

    cp->binary() = checkpoint_binary_flag;
    cp->write(fname);
  }
}
