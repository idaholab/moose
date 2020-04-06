//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

// Forward declarations

/**
 * Generate peridynamics mesh based on finite element mesh
 */
class MeshGeneratorPD : public MeshGenerator
{
public:
  static InputParameters validParams();

  MeshGeneratorPD(const InputParameters & parameters);

  /**
   * Function to convert the finite element mesh to peridynamics mesh
   * @return The converted mesh
   */
  std::unique_ptr<MeshBase> generate();

protected:
  /// Reference to the input finite element mesh
  std::unique_ptr<MeshBase> & _input;

  /// block ID(s) of input FE mesh when only certain block(s) needs to be converted to PD mesh
  /// this is used when there are listable to-be-converted blocks
  bool _has_conv_blk_ids;
  std::set<SubdomainID> _conv_blk_ids;

  /// block ID(s) of input FE mesh when only certain block(s) needs NOT to be converted to PD mesh
  /// this is usually used when there are considerable to-be-converted blocks but a few not to be converted
  bool _has_non_conv_blk_ids;
  std::set<SubdomainID> _non_conv_blk_ids;

  /// flag to specify whether the FE mesh should be retained or not
  /// in addition to newly created PD mesh
  bool _retain_fe_mesh;

  /// flag to specify whether to combine converted PD mesh blocks into a single mesh block or not
  /// this is used when all PD blocks have the same properties
  bool _single_converted_blk;

  /// flag to specify whether PD sideset should be constructed or not
  bool _construct_pd_sideset;

  /// pairs of converted FE block IDs when only certain blocks need to be connected using interfacial bonds
  /// this is used when there are listable to-be-connected blocks
  bool _has_connect_blk_id_pairs;
  std::multimap<SubdomainID, SubdomainID> _connect_blk_id_pairs;

  /// pairs of converted FE block IDs when only certain blocks need NOT to be connected
  /// this is usually used when there are considerable to-be-connected blocks but a few not to be connected
  bool _has_non_connect_blk_id_pairs;
  std::multimap<SubdomainID, SubdomainID> _non_connect_blk_id_pairs;

  /// flag to specify whether a single block should be used for all interfacial bonds
  /// this is used when all interfacial bonds have the same properties
  bool _single_interface_blk;
};
