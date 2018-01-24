//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DOMAININTEGRALACTION_H
#define DOMAININTEGRALACTION_H

// MOOSE includes
#include "Action.h"
#include "MooseEnum.h"
#include "MooseTypes.h"

#include "libmesh/vector_value.h"

class DomainIntegralAction;

template <>
InputParameters validParams<DomainIntegralAction>();

class DomainIntegralAction : public Action
{
public:
  DomainIntegralAction(const InputParameters & params);

  ~DomainIntegralAction();

  virtual void act();

protected:
  enum INTEGRAL
  {
    J_INTEGRAL,
    INTERACTION_INTEGRAL_KI,
    INTERACTION_INTEGRAL_KII,
    INTERACTION_INTEGRAL_KIII,
    INTERACTION_INTEGRAL_T
  };

  enum Q_FUNCTION_TYPE
  {
    GEOMETRY,
    TOPOLOGY
  };

  unsigned int calcNumCrackFrontPoints();

  std::set<INTEGRAL> _integrals;
  const std::vector<BoundaryName> & _boundary_names;
  std::vector<Point> _crack_front_points;
  bool _closed_loop;
  UserObjectName _crack_front_points_provider;
  bool _use_crack_front_points_provider;
  const std::string _order;
  const std::string _family;
  MooseEnum _direction_method_moose_enum;
  MooseEnum _end_direction_method_moose_enum;
  bool _have_crack_direction_vector;
  RealVectorValue _crack_direction_vector;
  bool _have_crack_direction_vector_end_1;
  RealVectorValue _crack_direction_vector_end_1;
  bool _have_crack_direction_vector_end_2;
  RealVectorValue _crack_direction_vector_end_2;
  std::vector<BoundaryName> _crack_mouth_boundary_names;
  std::vector<BoundaryName> _intersecting_boundary_names;
  bool _treat_as_2d;
  unsigned int _axis_2d;
  std::vector<Real> _radius_inner;
  std::vector<Real> _radius_outer;
  unsigned int _ring_first;
  unsigned int _ring_last;
  std::vector<VariableName> _output_variables;
  Real _poissons_ratio;
  Real _youngs_modulus;
  std::vector<SubdomainName> _blocks;
  std::vector<VariableName> _displacements;
  VariableName _temp;
  bool _convert_J_to_K;
  bool _has_symmetry_plane;
  unsigned int _symmetry_plane;
  MooseEnum _position_type;
  MooseEnum _q_function_type;
  bool _get_equivalent_k;
  bool _use_displaced_mesh;
  bool _output_q;
  std::vector<unsigned int> _ring_vec;
  bool _solid_mechanics;
};

#endif // DOMAININTEGRALACTION_H
