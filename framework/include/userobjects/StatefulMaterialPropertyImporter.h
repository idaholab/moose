//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "MaterialPropertyStorage.h"
#include "KDTree.h"

#include <map>

/**
 * Imports stateful material property data from a binary file (.smatprop) written
 * by StatefulMaterialPropertyExporter and remaps it onto the current mesh using
 * closest-point matching within each subdomain.
 *
 * Timing: execute_on = EXEC_INITIAL, which fires AFTER FEProblemBase::initialSetup()
 * completes (including initElementStatefulProps()). initStatefulProperties() runs
 * normally for ALL elements first. The importer then overwrites only states 1 (old)
 * and 2 (older) for elements/properties present in the import file. State 0 (current)
 * is left as-is and recomputed in the first timestep.
 *
 * This correctly handles partial imports: materials with both imported and non-imported
 * properties will have all properties properly initialized via initStatefulProperties(),
 * with only the imported ones subsequently overwritten.
 */
class StatefulMaterialPropertyImporter : public GeneralUserObject
{
public:
  static InputParameters validParams();

  StatefulMaterialPropertyImporter(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// The file to read from
  const std::string & _file;

  /// Property metadata from file
  struct FilePropRecord
  {
    std::string name;
    std::string type_str;
    unsigned int max_state;
  };
  std::vector<FilePropRecord> _file_props;

  /// Per-qp stored data
  struct StoredQpRecord
  {
    Point coord;
    /// blobs[stateful_id][state] = binary blob for one qp value
    std::vector<std::vector<std::string>> blobs;
  };

  /// Stored data organized by subdomain name
  std::map<std::string, std::vector<StoredQpRecord>> _stored_data;

  /// Per-subdomain KDTree and point list
  std::map<std::string, std::unique_ptr<KDTree>> _kdtrees;
  std::map<std::string, std::vector<Point>> _kdtree_points;

  /// Mapping from file stateful_id to current simulation stateful_id
  std::vector<std::optional<unsigned int>> _file_to_current_sid;

  /// Read the binary file
  void readFile();

  /// Build one KDTree per subdomain
  void buildKDTrees();

  /// Build the property name mapping from file IDs to current simulation stateful IDs
  void buildPropertyMapping();

  /// Overwrite states 1 and 2 in _storage with remapped data from the file
  void overwriteStorageStates();
};
