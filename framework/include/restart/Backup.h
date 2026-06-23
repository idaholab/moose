//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <sstream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/**
 * Helper class to hold streams for Backup and Restore operations.
 *
 * The header and data streams hold restartable/recoverable data. The optional mesh file entries
 * hold an in-memory copy of libMesh checkpoint files for adapted meshes whose topology must be
 * restored before loading equation-system data.
 */
struct Backup
{
  /// Restartable data header stream.
  std::unique_ptr<std::stringstream> header = std::make_unique<std::stringstream>();
  /// Restartable data payload stream.
  std::unique_ptr<std::stringstream> data = std::make_unique<std::stringstream>();
  /// Pairs of checkpoint-relative file names and binary file contents.
  std::vector<std::pair<std::string, std::string>> mesh_files;
};

void dataStore(std::ostream & stream, Backup & backup, void * context);
void dataLoad(std::istream & stream, Backup & backup, void * context);
void dataStore(std::ostream & stream, std::unique_ptr<Backup> & backup, void * context);
void dataLoad(std::istream & stream, std::unique_ptr<Backup> & backup, void * context);
