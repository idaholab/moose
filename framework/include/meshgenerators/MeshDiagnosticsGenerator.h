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
#include "MooseEnum.h"

/*
 * Mesh 'generator' to diagnose potentially unsupported features or miscellaneous issues
 */
class MeshDiagnosticsGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  MeshDiagnosticsGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// the input mesh to be diagnosed
  std::unique_ptr<MeshBase> & _input;

private:
  /**
   * Utility routine to output the final diagnostics level in the desired mode
   * @param msg the message to output
   * @param log_level the log level to output the message at
   * @param may_error if set to false, prevents erroring from the log, despite the log level
   * may_error is used to avoid erroring when the log is requested but there are no issues so it
   * should just say "0 problems" with an info message
   */
  void diagnosticsLog(std::string msg, const MooseEnum & log_level, bool may_error);

  /// whether to check element volumes
  const MooseEnum _check_element_volumes;
  /// counter for the number of small elements
  unsigned int _num_tiny_elems;
  /// counter for the number of big elements
  unsigned int _num_big_elems;
  /// minimum size for element volume to be counted as a tiny element
  const Real _min_volume;
  /// maximum size for element volume to be counted as a big element
  const Real _max_volume;
  /// whether to check different element types in the same sub-domain
  const MooseEnum _check_element_types;
  /// whether to check for intersecting elements
  const MooseEnum _check_element_overlap;
  /// whether to check for number of elements overlapping
  unsigned int _num_elem_overlaps;
  /// whether to check for elements in different planes (non_planar)
  const MooseEnum _check_non_planar_sides;
  /// counter for the number of sides that are non-planar
  unsigned int _sides_non_planar;
  /// whether to check for non-conformal meshes
  const MooseEnum _check_non_conformal_mesh;
  /// tolerance for detecting when meshes are not conformal
  const Real _non_conformality_tol;
  /// counter for the number of non-conformal elements
  unsigned int _num_nonconformal_nodes;

  /// whether to check for the adaptivity of non-conformal meshes
  const MooseEnum _check_adaptivity_non_conformality;
};
