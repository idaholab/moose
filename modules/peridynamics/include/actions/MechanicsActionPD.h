//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Action class to setup peridynamic models for solid mechanics problems
 */
class MechanicsActionPD : public Action
{
public:
  static InputParameters validParams();

  MechanicsActionPD(const InputParameters & params);

  virtual void act() override;

protected:
  /**
   * Function to get the kernel name based on the value of member variables:
   * _formulation and _stabilization
   * @return Kernel name
   */
  virtual std::string getKernelName();

  /**
   * Function to get the input parameters for a given kernel name
   * @param name   the name of the kernel
   * @return Parameters for the corresponding kernel
   */
  virtual InputParameters getKernelParameters(std::string name);

  ///@{ Displacement variables
  std::vector<VariableName> _displacements;
  const unsigned int _ndisp;
  ///@}

  /// Option of which peridynamic model needs to be setup:
  /// BOND, ORDINARY_STATE or NONORDINARY_STATE
  const MooseEnum _formulation;

  /// Option of stabilization scheme for correspondence material model:
  /// FORCE, BOND_HORIZON_I or BOND_HORIZON_II
  const MooseEnum _stabilization;

  /// Option of strain formulation: SMALL or FINITE
  const MooseEnum _strain;

  /// vector of subdomain names from provided blocks
  std::vector<SubdomainName> _subdomain_names;

  /// set of subdomain IDs generated from the passed in vector of subdomain names
  std::set<SubdomainID> _subdomain_ids;

  /// set of SubdomainID generated from the combined block restrictions of
  /// all TensorMechanics/Master action blocks
  std::set<SubdomainID> _subdomain_id_union;

  ///@{ Residual debugging
  std::vector<AuxVariableName> _save_in;
  std::vector<AuxVariableName> _diag_save_in;
  ///@}
};
