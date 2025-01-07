//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ConnectorBase.h"

class ClosuresBase;

/**
 * Base class for scalar transfer connections
 */
class ScalarTransferBase : public ConnectorBase
{
public:
  static InputParameters validParams();

  ScalarTransferBase(const InputParameters & parameters);

  /**
   * Returns wall scalar flux names for scalar of index i
   * @param scalar_i the index of the scalar flux
   * @return The name of wall scalar flux functor
   */
  const MooseFunctorName & getWallScalarFluxName(unsigned int scalar_i) const;

  /// Get the names of all the scalar fluxes
  const std::vector<MooseFunctorName> & getWallScalarFluxNames() const
  {
    return _wall_scalar_flux_names;
  }

  /// Get the names of all the scalar wall values
  virtual const std::vector<MooseFunctorName> & getWallScalarValuesNames() const
  {
    mooseError("Not implemented");
  }

  /**
   * Get the list of the subdomain names associated with the flow channel
   *
   * @return List of subdomain names associated with the flow channel
   */
  const std::vector<SubdomainName> & getFlowChannelSubdomains() const
  {
    return _flow_channel_subdomains;
  }

  const UserObjectName & getFluidPropertiesName() const;

  /**
   * Get the name of the connected flow channel
   */
  const std::string & getFlowChannelName() const { return _flow_channel_name; }

protected:
  virtual void init() override;
  virtual void initSecondary() override;
  virtual void check() const override;

  /// name of the connected flow channel
  const std::string _flow_channel_name;

  /// Names of the passive scalars being transferred
  std::vector<VariableName> _passive_scalar_names;

  /// Used closures
  std::shared_ptr<ClosuresBase> _closures;

  /// wall scalar flux name
  std::vector<MooseFunctorName> _wall_scalar_flux_names;

  /// Subdomains corresponding to the connected flow channel
  std::vector<SubdomainName> _flow_channel_subdomains;
  /// fluid properties object name
  UserObjectName _fp_name;
  /// area function name for the connected flow channel
  FunctionName _A_fn_name;
};
