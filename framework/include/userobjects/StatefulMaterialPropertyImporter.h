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
#include "StatefulMaterialPropertyExporter.h"
#include "KDTree.h"

#include <array>
#include <map>

/**
 * Imports stateful material property data from a binary file (.smatprop) written
 * by StatefulMaterialPropertyExporter and remaps it onto the current mesh using
 * closest-point matching. Volumetric data is matched within each subdomain; boundary data is
 * matched within each group of element sides that share the subdomain, the set of boundary
 * names, and the neighbor subdomain (see StatefulMaterialPropertyExporter::GroupKey).
 *
 * Timing: execute_on = EXEC_INITIAL. execute() runs during FEProblemBase::initialSetup()
 * after the first initElementStatefulProps() pass but before a second pass that reinitializes
 * stateful properties. The importer uses execute() to populate
 * MaterialPropertyStorage::_restartable_map so that the second initStatefulProps() call
 * loads remapped values in place while non-imported properties keep the values established
 * during the first initialization pass.
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
  /// Base name for the set of .smatprop files (rank suffix is appended automatically)
  const std::string & _file_base;

  using GroupKey = StatefulMaterialPropertyExporter::GroupKey;

  /// Property metadata from file
  struct FilePropRecord
  {
    std::string name;
    std::string type_str;
    unsigned int max_state;
  };

  /// Per-qp stored data
  struct StoredQpRecord
  {
    Point coord;
    /// blobs[stateful_id][state] = binary blob for one qp value
    std::vector<std::vector<std::string>> blobs;
  };

  /// Data read from file for one material property storage (volumetric or boundary)
  struct StorageData
  {
    /// Property metadata from file
    std::vector<FilePropRecord> file_props;
    /// Stored data organized by group
    std::map<GroupKey, std::vector<StoredQpRecord>> stored_data;
    /// Per-group KDTree and point list
    std::map<GroupKey, std::unique_ptr<KDTree>> kdtrees;
    std::map<GroupKey, std::vector<Point>> kdtree_points;
    /// Mapping from file stateful_id to current simulation stateful_id
    std::vector<std::optional<unsigned int>> file_to_current_sid;
  };

  /// Imported volumetric (index 0) and boundary (index 1) data, in file order
  std::array<StorageData, 2> _storage_data;

  /// The storage that _storage_data[\p index] is imported into
  MaterialPropertyStorage & remapStorage(unsigned int index);

  /// Read all rank files ({base}.0.smatprop … {base}.{n_ranks-1}.smatprop)
  void readAllFiles();

  /// Read and merge one rank file into _storage_data
  void readSingleFile(const std::string & filename, bool first);

  /// Read and merge the section for one storage from \p in into \p data
  void readStorage(std::istream & in, const std::string & filename, StorageData & data, bool first);

  /// Build one KDTree per group
  void buildKDTrees(StorageData & data);

  /// Build the property name mapping from file IDs to current simulation stateful IDs
  void buildPropertyMapping(StorageData & data, const MaterialPropertyStorage & storage);

  /// Populate MaterialPropertyStorage::_restartable_map with remapped data for later loading
  void populateRestartableMap();

  /**
   * Stage remapped data for the quadrature points \p q_points of side \p side of \p elem
   * (side 0 for volumetric data) from the group \p key into \p storage
   */
  void stageRemappedData(const StorageData & data,
                         MaterialPropertyStorage & storage,
                         const GroupKey & key,
                         const Elem * elem,
                         unsigned int side,
                         const MooseArray<Point> & q_points);
};
