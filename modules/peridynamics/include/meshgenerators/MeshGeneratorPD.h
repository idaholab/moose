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
  /// a number used to offset the block ID after being converted into PD mesh
  unsigned int _pd_blk_offset_number = 1000;

  /// a number used to offset the block ID for phantom elements
  unsigned int _phantom_blk_offset_number = 10000;

  /// a number used to offset the boundary nodeset ID after being converted into PD nodeset
  unsigned int _pd_nodeset_offset_number = 999;

  /// Reference to the input finite element mesh
  std::unique_ptr<MeshBase> & _input;

  /// block ID(s) of input FE mesh when only certain block(s) needs to be converted to PD mesh
  /// this is used when there are listable to-be-converted blocks
  bool _has_blks_to_pd;
  std::set<SubdomainID> _blks_to_pd;

  /// block ID(s) of input FE mesh when only certain block(s) needs NOT to be converted to PD mesh
  /// this is usually used when there are considerable to-be-converted blocks but a few not to be converted
  bool _has_blks_as_fe;
  std::set<SubdomainID> _blks_as_fe;

  /// flag to specify whether the FE mesh should be retained or not
  /// in addition to newly created PD mesh
  bool _retain_fe_mesh;

  /// flag to specify whether to combine converted PD mesh blocks into a single mesh block or not
  /// this is used when all PD blocks have the same properties
  bool _merge_pd_blks;

  /// flag to specify whether PD sideset should be constructed or not
  bool _construct_pd_sideset;
  /// list of sideset ID(s) to be constructed based on converted PD mesh if the _construct_pd_sideset is true
  bool _has_sidesets_to_pd;
  std::set<boundary_id_type> _fe_sidesets_for_pd_construction;

  /// pairs of converted FE block IDs when only certain blocks need to be connected using interfacial bonds
  /// this is used when there are listable to-be-connected blocks
  bool _has_bonding_blk_pairs;
  std::multimap<SubdomainID, SubdomainID> _pd_bonding_blk_pairs;

  /// pairs of converted FE block IDs when only certain blocks need NOT to be connected
  /// this is usually used when there are considerable to-be-connected blocks but a few not to be connected
  bool _has_non_bonding_blk_pairs;
  std::multimap<SubdomainID, SubdomainID> _pd_non_bonding_blk_pairs;

  /// flag to specify whether a single block should be used for all PD interfacial blocks
  /// this is used when all interfacial bonds have the same properties
  bool _merge_pd_interfacial_blks;
};
