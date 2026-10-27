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
#include "FunctionParserUtils.h"
#include "NonADFunctorInterface.h"
#include "MooseTypes.h"

#include <unordered_map>
#include <vector>

class Function;
class Assembly;
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;

/**
 * MeshModifier that actively displaces the selected nodes by three parsed
 * expressions (x, y, z displacement components), evaluated relative to each node's
 * original (undisplaced) position so repeated execution does not accumulate drift.
 *
 * Unlike MoveNodesToGeometryModifierBase, this object modifies the mesh on its own
 * execution schedule (execute_on) rather than in response to the problem's
 * meshChanged() event. It can optionally notify the problem that the mesh changed
 * after moving the nodes.
 */
class MoveNodesByParsedExpressionModifier : public GeneralUserObject,
                                            public FunctionParserUtils<false>,
                                            public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  MoveNodesByParsedExpressionModifier(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// Displace all selected nodes (boundary/block restricted, or all blocks)
  void moveNodes();

  /// Displace a single node by the parsed expressions, relative to its original position.
  /// When \p owner_only is true, only the node's owning rank computes the displacement;
  /// moveNodes() then synchronizes the resulting positions.
  void displaceNode(Node & node, bool owner_only);

  /// Capture the original element volumes (once) before moving any nodes
  void prepare();

  /// Write the optional original-coordinate, displacement, and density-factor aux variables
  void writeOutputs();

  /// Validate a list of three nodal output aux variable names (exactly three,
  /// each existing and nodal) and cache their variable numbers
  void setupNodalOutputVariables(const std::string & param_name,
                                 const std::vector<AuxVariableName> & names,
                                 std::vector<unsigned int> & var_numbers);

  /// Reference to the current simulation mesh
  MooseMesh & _mesh;
  /// Boundaries whose nodes are displaced
  const std::vector<BoundaryID> _boundary_ids;
  /// Blocks whose nodes are displaced
  const std::vector<SubdomainID> _subdomain_ids;

  /// Parameter names of the three displacement expressions (x, y, z)
  static const std::string _disp_name[3];

  /// The three parsed displacement functions (x, y, z)
  SymFunctionPtr _displacement[3];

  /// Coupled (nodal) variables referenced in the expressions, in symbol order
  std::vector<const MooseVariable *> _coupled_vars;

  /// Functions referenced in the expressions, in symbol order
  std::vector<const Function *> _functions;

  /// Postprocessor values referenced in the expressions, in symbol order
  std::vector<const PostprocessorValue *> _postprocessors;

  /// Functors referenced in the expressions, in symbol order. Evaluated at the node in its
  /// reference (undisplaced) configuration
  std::vector<const Moose::Functor<Real> *> _functors;

  /// Original (reference) position of each node, captured on first touch. Restartable so
  /// that a recovered run keeps the undisplaced reference; the checkpointed mesh is already
  /// displaced, so re-capturing it here would compound the displacement.
  std::unordered_map<dof_id_type, Point> & _original_position;

  ///@{ Optional outputs, each enabled only when its parameter is provided
  const bool _output_coordinates;
  const bool _output_displacements;
  const bool _output_density_factor;
  ///@}

  /// Auxiliary system number (set when any output is enabled)
  unsigned int _aux_sys_num;
  /// Aux variable numbers of the original-coordinate components (x, y, z)
  std::vector<unsigned int> _coordinate_var;
  /// Aux variable numbers of the displacement components (x, y, z)
  std::vector<unsigned int> _displacement_var;
  /// Aux variable number of the per-element density adjustment factor
  unsigned int _density_factor_var;

  /// Assembly used to compute coordinate-aware element volumes (density factor)
  Assembly * _assembly;
  /// Original (undisplaced) coordinate-aware element volumes, captured once. Restartable
  /// for the same reason as _original_position.
  std::unordered_map<dof_id_type, Real> & _original_volume;
  /// Whether the original element volumes have been recorded yet
  bool & _original_volume_recorded;

  /// Whether to notify the problem that the mesh changed after moving nodes
  const bool _notify_mesh_changed;

  usingFunctionParserUtilsMembers(false);
};
