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

/**
 * Base class for computing and caching flux and residual vectors for a volume junction
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the junction, and
 * \li fluxes between the flow channels and the junction.
 */
class ADVolumeJunctionBaseUserObject : public ADFlowJunctionUserObject
{
public:
  /**
   * Constructor.
   *
   * @param[in] params                  Input parameters
   * @param[in] flow_variable_names     Vector of coupled flow variable names,
   *                                    indexed by equation index
   * @param[in] scalar_variable_names   Vector of coupled scalar variable names,
   *                                    indexed by equation index
   */
  ADVolumeJunctionBaseUserObject(const InputParameters & params);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

  /**
   * Returns the residual vector for the scalar variables
   */
  const std::vector<ADReal> & getResidual() const;

  const std::vector<ADReal> & getFlux(const unsigned int & connection_index) const override;

protected:
  /**
   * Stores data (connection index, face shape functions, DoFs associated with flow channel
   * variables) related to a connection
   *
   * Should be called in execute()
   */
  virtual void storeConnectionData();

  /**
   * Computes and stores the fluxes, the scalar residuals, and their Jacobians.
   *
   * @param[in] c   Connection index
   */
  virtual void computeFluxesAndResiduals(const unsigned int & c) = 0;

  /// Volume of the junction
  const Real & _volume;

  /// Vector of coupled variable names for each flow variable
  std::vector<std::string> _flow_variable_names;
  /// Vector of coupled variable names for each scalar variable
  std::vector<std::string> _scalar_variable_names;

  /// Number of flow channel flux components
  unsigned int _n_flux_eq;
  /// Number of scalar residual components
  unsigned int _n_scalar_eq;

  /// Names of numerical flux user objects for each connected flow channel
  const std::vector<UserObjectName> & _numerical_flux_names;

  /// Connection indices for this thread
  std::vector<unsigned int> _connection_indices;

  /// Degrees of freedom for scalar variables
  std::vector<dof_id_type> _scalar_dofs;
  /// Degrees of freedom for flow channel variables, for each connection
  std::vector<std::vector<dof_id_type>> _flow_channel_dofs;

  /// Cached flux vector for each connection
  std::vector<std::vector<ADReal>> _flux;

  /// Cached scalar residual vector
  std::vector<ADReal> _residual;

public:
  static InputParameters validParams();
};
