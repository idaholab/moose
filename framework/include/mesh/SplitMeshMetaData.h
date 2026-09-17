//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <filesystem>
#include <string>
#include <vector>

class MooseApp;

/**
 * Stores and checks the mesh input fingerprint attached to pre-split mesh metadata.
 */
class SplitMeshMetaData
{
public:
  explicit SplitMeshMetaData(MooseApp & app);

  /**
   * Writes the current mesh input fingerprint and summary for split mesh output.
   *
   * @return The paths that were written
   */
  std::vector<std::filesystem::path> write(const std::filesystem::path & folder_base);

  /**
   * Checks that the mesh input fingerprint stored with a pre-split mesh matches this run.
   */
  void check(const std::filesystem::path & folder_base);

private:
  /// Computes the current canonical mesh input summary used for the split mesh fingerprint
  std::string inputSummary();

  /// Computes the current mesh input fingerprint from a canonical summary
  std::string inputFingerprint(const std::string & summary);

  /// Returns the metadata location for split mesh fingerprint storage and checking
  std::filesystem::path metaDataFolderBase(const std::filesystem::path & folder_base);

  /// The application whose mesh input and metadata are being checked.
  MooseApp & _app;
};
