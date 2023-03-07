//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Action.h"
#include "MooseEnum.h"
#include "MooseTypes.h"

#include "libmesh/vector_value.h"

/**
 * Action to set up all objects used in computation of fracture domain integrals
 */
class DomainIntegralAction : public Action
{
public:
  static InputParameters validParams();

  DomainIntegralAction(const InputParameters & params);

  ~DomainIntegralAction();

  virtual void act() override;

  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

protected:
  /// Enum used to select the type of integral to be performed
  enum INTEGRAL
  {
    J_INTEGRAL,
    C_INTEGRAL,
    K_FROM_J_INTEGRAL,
    INTERACTION_INTEGRAL_KI,
    INTERACTION_INTEGRAL_KII,
    INTERACTION_INTEGRAL_KIII,
    INTERACTION_INTEGRAL_T
  };

  /// Enum used to select the method used to compute the Q function used
  /// in the fracture integrals
  enum Q_FUNCTION_TYPE
  {
    GEOMETRY,
    TOPOLOGY
  };

  /**
   * Compute the number of points on the crack front. This is either the number of
   * points in the crack front nodeset, or the number of points from the crack
   * front points provider.
   * @return Number of points on the crack front
   */
  unsigned int calcNumCrackFrontPoints();

  /// Container for enumerations describing the individual integrals computed.
  std::set<INTEGRAL> _integrals;
  /// Boundaries containing the crack front points
  const std::vector<BoundaryName> & _boundary_names;
  /// User-defined vector of crack front points
  std::vector<Point> _crack_front_points;
  /// Indicates whether the crack forms a closed loop
  bool _closed_loop;
  /// Name of crack front points provider user object used to optionally define the crack points
  UserObjectName _crack_front_points_provider;
  /// Whether to use a crack front points provider
  bool _use_crack_front_points_provider;
  /// Order and family of the AuxVariables optionally created to output the values of q
  ///@{
  const std::string _order;
  const std::string _family;
  ///@}
  /// Enum used to define the method to compute crack front direction
  MooseEnum _direction_method_moose_enum;
  /// Enum used to define the method to compute crack front direction at ends of crack front
  MooseEnum _end_direction_method_moose_enum;
  /// Whether the crack direction vector has been provided
  const bool _have_crack_direction_vector;
  /// Vector optionally used to prescribe direction of crack extension
  const RealVectorValue _crack_direction_vector;
  /// Whether the crack direction vector at the 1st end of the crack has been provided
  const bool _have_crack_direction_vector_end_1;
  /// Vector optionally used to prescribe direction of crack extension at the 1st end of the crack
  const RealVectorValue _crack_direction_vector_end_1;
  /// Whether the crack direction vector at the 2nd end of the crack has been provided
  const bool _have_crack_direction_vector_end_2;
  /// Vector optionally used to prescribe direction of crack extension at the 2nd end of the crack
  const RealVectorValue _crack_direction_vector_end_2;
  /// Names of boundaries optionally used to define the crack mouth location
  std::vector<BoundaryName> _crack_mouth_boundary_names;
  /// Names of boundaries optionally used for improved computation of crack extension direction at
  /// ends of the crack where it intersects the prescribed boundaries.
  std::vector<BoundaryName> _intersecting_boundary_names;
  /// Whether fracture computations for a 3D model should be treated as though it were a 2D model
  bool _treat_as_2d;
  /// Out-of-plane axis for 3D models treated as 2D
  unsigned int _axis_2d;
  /// Sets of inner and outer radii of the rings used for the domain form of the fracture integrals.
  /// These are defined in corresponding pairs for each ring.
  ///@{
  std::vector<Real> _radius_inner;
  std::vector<Real> _radius_outer;
  ///@}
  /// Number of elements away from the crack tip to inside of inner ring with the topological q function
  unsigned int _ring_first;
  /// Number of elements away from the crack tip to outside of outer ring with the topological q function
  unsigned int _ring_last;
  /// List of variables for which values are to be sampled and output at the crack front points
  std::vector<VariableName> _output_variables;
  /// Poisson's ratio of material
  Real _poissons_ratio;
  /// Young's modulus of material
  Real _youngs_modulus;
  /// Blocks for which the domain integrals are to be computed
  std::vector<SubdomainName> _blocks;
  /// Vector of displacement variables
  std::vector<VariableName> _displacements;
  /// Temperature variable
  VariableName _temp;
  /// Whether the model has a symmetry plane passing through the plane of the crack
  bool _has_symmetry_plane;
  /// Identifier for which plane is the symmetry plane
  unsigned int _symmetry_plane;
  /// How the distance along the crack front is measured (angle or distance)
  MooseEnum _position_type;
  /// How the q function is evaluated (geometric distance from crack front or ring of elements)
  MooseEnum _q_function_type;
  /// Whether to compute the equivalent K from the individual fracture integrals for mixed-mode fracture
  bool _get_equivalent_k;
  /// Whether to compute the fracture integrals on the displaced mesh
  bool _use_displaced_mesh;
  /// Whether to ouput the q function as a set of AuxVariables
  bool _output_q;
  /// Vector of ids for the individual rings on which the fracture integral is computed
  std::vector<unsigned int> _ring_vec;
  /// Whether the constitutive models for the mechanics calculations use an incremental form
  bool _incremental;
  /// Whether to convert the J-integral to a stress intensity factor (K) --deprecated
  bool _convert_J_to_K;
  /// Whether the crack lives in a functionally-graded material
  bool _fgm_crack;
  /// Material property name for the Youngs modulus derivative for functionally graded materials
  MaterialPropertyName _functionally_graded_youngs_modulus_crack_dir_gradient;
  /// Material property name for  spatially-dependent Youngs modulus for functionally graded materials
  MaterialPropertyName _functionally_graded_youngs_modulus;
  /// Whether to create automatic differentiation objects from the action
  const bool _use_ad;
};
