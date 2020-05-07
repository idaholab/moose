/*************************************************/
/*           DO NOT MODIFY THIS HEADER           */
/*                                               */
/*                 Rattlesnake                   */
/*                                               */
/*    (c) 2017 Battelle Energy Alliance, LLC     */
/*            ALL RIGHTS RESERVED                */
/*                                               */
/*   Prepared by Battelle Energy Alliance, LLC   */
/*     Under Contract No. DE-AC07-05ID14517      */
/*     With the U. S. Department of Energy       */
/*                                               */
/*     See COPYRIGHT for full restrictions       */
/*************************************************/

#pragma once

#include "GeneratedMeshGenerator.h"

class GeneratedIDMeshGenerator;

template <>
InputParameters validParams<GeneratedIDMeshGenerator>();

/**
 * This object extends GeneratedMeshGenerator by adding element IDs including subdomain IDs
 * to each element. In particular, material, depletion and equivalence IDs are
 * added based on the user's input.
 */

class GeneratedIDMeshGenerator : public GeneratedMeshGenerator
{
public:
  static InputParameters validParams();

  GeneratedIDMeshGenerator(const InputParameters & params);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// whether subdomain IDs are needed
  bool _has_subdomain_id;
  /// whether material IDs are needed
  bool _has_material_id;
  /// whether depletion IDs are needed
  bool _has_depletion_id;
  /// whether equivalence IDs are needed
  bool _has_equivalence_id;
};
