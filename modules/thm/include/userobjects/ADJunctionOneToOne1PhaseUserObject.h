#pragma once

#include "ADFlowJunctionUserObject.h"

class ADNumericalFlux3EqnBase;

/**
 * Computes flux between two subdomains for 1-phase one-to-one junction
 */
class ADJunctionOneToOne1PhaseUserObject : public ADFlowJunctionUserObject
{
public:
  ADJunctionOneToOne1PhaseUserObject(const InputParameters & params);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

  const std::vector<ADReal> & getFlux(const unsigned int & connection_index) const override;

protected:
  /// Cross-sectional area of connected flow channels
  const ADVariableValue & _A;
  /// rho*A of the connected flow channels
  const ADVariableValue & _rhoA;
  /// rho*u*A of the connected flow channels
  const ADVariableValue & _rhouA;
  /// rho*E*A of the connected flow channels
  const ADVariableValue & _rhoEA;

  /// Numerical flux user object
  const ADNumericalFlux3EqnBase & _numerical_flux;

  /// Name of junction component
  const std::string & _junction_name;

  /// Direction material property
  const ADMaterialProperty<RealVectorValue> & _dir;

  /// Solution vectors for each connection
  std::vector<std::vector<ADReal>> _solutions;
  /// Flux vector
  std::vector<std::vector<ADReal>> _fluxes;
  /// Degree of freedom indices; first index is connection, second is equation
  std::vector<std::vector<dof_id_type>> _dof_indices;

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

  /// Thread lock
  static Threads::spin_mutex _spin_mutex;

public:
  static InputParameters validParams();
};
