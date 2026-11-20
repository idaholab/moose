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

#include <tuple>

class MaterialPropertyStorage;
class MooseMesh;

/**
 * Exports volumetric and boundary stateful material property data along with
 * quadrature point positions and grouping information to a binary file (.smatprop).
 * This data can be loaded by StatefulMaterialPropertyImporter on a
 * different mesh to remap stateful properties using closest-point matching.
 */
class StatefulMaterialPropertyExporter : public GeneralUserObject
{
public:
  static InputParameters validParams();

  StatefulMaterialPropertyExporter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

  /**
   * Identifies a group of quadrature points that are matched against each other on import:
   * the element's subdomain name, the sorted names of the boundaries that contain the element
   * side, and the subdomain name of the neighbor across the side. The last two are empty for
   * volumetric data, and the neighbor subdomain name is empty for sides without a neighbor.
   */
  using GroupKey = std::tuple<std::string, std::vector<std::string>, std::string>;

  /// The group key for volumetric data on \p elem
  static GroupKey volumeKey(const MooseMesh & mesh, const Elem & elem);
  /// The group key for face data on side \p side of \p elem
  static GroupKey faceKey(const MooseMesh & mesh, const Elem & elem, unsigned int side);

  /// File format identifier ("MPMS" in ASCII)
  static constexpr unsigned int file_magic = 0x4D504D53;
  /// File format version; version 2 added the boundary material property section
  static constexpr unsigned int file_version = 2;

protected:
  /**
   * Write the property metadata and grouped per-qp data of \p storage to \p out
   * @param face Whether \p storage holds face (boundary) data rather than volumetric data
   */
  void writeStorage(std::ostream & out, const MaterialPropertyStorage & storage, bool face);

  /// The file base name for output
  const std::string & _file_base;
};
