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
 * This UserObject does its work in initialSetup(), which runs BEFORE
 * initElementStatefulProps(). It populates MaterialPropertyStorage::_restartable_map
 * so that initStatefulProps() finds entries for each element and loads the remapped
 * data instead of calling initStatefulProperties().
 */
class StatefulMaterialPropertyImporter : public GeneralUserObject
{
public:
  static InputParameters validParams();

  StatefulMaterialPropertyImporter(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override {}
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

  /// Read the binary file
  void readFile();

  /// Build one KDTree per subdomain
  void buildKDTrees();

  /// Populate _restartable_map with remapped data
  void populateRestartableMap();
};
