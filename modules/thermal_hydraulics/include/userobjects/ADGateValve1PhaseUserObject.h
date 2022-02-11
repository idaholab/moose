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

class ADNumericalFlux3EqnBase;

/**
 * Gate valve user object for 1-phase flow
 */
class ADGateValve1PhaseUserObject : public ADFlowJunctionUserObject
{
public:
  ADGateValve1PhaseUserObject(const InputParameters & params);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

  const std::vector<ADReal> & getFlux(const unsigned int & connection_index) const override;

protected:
  /// Fraction of possible flow area that is open
  const Real & _f_open;
  /// Minimum open area fraction
  const Real & _f_open_min;

  /// Cross-sectional area of connected flow channels
  const ADVariableValue & _A;
  /// rho*A of the connected flow channels
  const ADVariableValue & _rhoA;
  /// rho*u*A of the connected flow channels
  const ADVariableValue & _rhouA;
  /// rho*E*A of the connected flow channels
  const ADVariableValue & _rhoEA;

  /// Flow channel rho*A coupled variable index
  const unsigned int _rhoA_jvar;
  /// Flow channel rho*u*A coupled variable index
  const unsigned int _rhouA_jvar;
  /// Flow channel rho*E*A coupled variable index
  const unsigned int _rhoEA_jvar;

  /// Pressure material property
  const ADMaterialProperty<Real> & _p;

  /// Numerical flux user object
  const ADNumericalFlux3EqnBase & _numerical_flux;

  /// Name of the associated component
  const std::string & _component_name;

  /// Direction material property
  const MaterialProperty<RealVectorValue> & _dir;

  /// Solution vectors for each connection
  std::vector<std::vector<ADReal>> _solutions;
  /// Flux vector
  std::vector<std::vector<ADReal>> _fluxes;
  /// Degree of freedom indices; first index is connection, second is equation
  std::vector<std::vector<dof_id_type>> _dof_indices;

  /// Stored pressure for each connection
  std::vector<ADReal> _stored_p;

  /// Element IDs for each connection
  std::vector<unsigned int> _elem_ids;
  /// Local side IDs for each connection
  std::vector<unsigned int> _local_side_ids;

  /// Areas at each connection
  std::vector<ADReal> _areas;
  /// Directions at each connection
  std::vector<RealVectorValue> _directions;

  /// Connection indices for this thread
  std::vector<unsigned int> _connection_indices;

  /// Pairs of variable names vs. their corresponding equation indices
  static const std::vector<std::pair<std::string, unsigned int>> _varname_eq_index_pairs;

public:
  static InputParameters validParams();
};
