//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADFlowJunctionUserObject.h"
#include "SlopeReconstruction1DInterface.h"

class ADNumericalFlux3EqnBase;
class SinglePhaseFluidProperties;

/**
 * Computes flux between two subdomains for 1-phase one-to-one junction
 */
class ADJunctionOneToOne1PhaseUserObject : public ADFlowJunctionUserObject,
                                           public SlopeReconstruction1DInterface<true>
{
public:
  ADJunctionOneToOne1PhaseUserObject(const InputParameters & params);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

  const std::vector<ADReal> & getFlux(const unsigned int & connection_index) const override;

  virtual std::vector<ADReal> computeElementPrimitiveVariables(const Elem * elem) const override;

protected:
  /// Piecewise linear cross-sectional area of connected flow channels
  const ADVariableValue & _A_linear;

  // piecewise constant conserved variables
  MooseVariable * _A_var;
  MooseVariable * _rhoA_var;
  MooseVariable * _rhouA_var;
  MooseVariable * _rhoEA_var;
  /// Conservative solution variables
  std::vector<MooseVariable *> _U_vars;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;

  /// Numerical flux user object
  const ADNumericalFlux3EqnBase & _numerical_flux;

  /// Name of junction component
  const std::string & _junction_name;

  /// Direction material property
  const MaterialProperty<RealVectorValue> & _dir;

  /// Primitive solution vectors for each connection
  std::vector<std::vector<ADReal>> _primitive_solutions;
  /// Primitive solution vectors for each connection's neighbor
  std::vector<std::vector<ADReal>> _neighbor_primitive_solutions;

  /// Flux vector
  std::vector<std::vector<ADReal>> _fluxes;
  /// Degree of freedom indices; first index is connection, second is equation
  std::vector<std::vector<dof_id_type>> _dof_indices;

  /// Element IDs for each connection
  std::vector<unsigned int> _elem_ids;
  /// Local side IDs for each connection
  std::vector<unsigned int> _local_side_ids;

  /// Piecewise linear areas at each connection
  std::vector<ADReal> _areas_linear;
  /// Directions at each connection
  std::vector<RealVectorValue> _directions;
  /// Positions at each connection
  std::vector<Point> _positions;
  /// Positions at each connection's neighbor
  std::vector<Point> _neighbor_positions;
  /// Position changes for each connection
  std::vector<Real> _delta_x;
  /// Flags for each connection having a neighbor
  std::vector<bool> _has_neighbor;

  /// Connection indices for this thread
  std::vector<unsigned int> _connection_indices;

  /// Pairs of variable names vs. their corresponding equation indices
  static const std::vector<std::pair<std::string, unsigned int>> _varname_eq_index_pairs;

  /// Thread lock
  static Threads::spin_mutex _spin_mutex;

public:
  static InputParameters validParams();
};
