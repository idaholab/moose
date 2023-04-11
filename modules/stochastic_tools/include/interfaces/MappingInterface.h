//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "FEProblemBase.h"
#include "VariableMappingBase.h"

/**
 * An interface class that helps getting access to Mapping objects
 */
class MappingInterface
{
public:
  static InputParameters validParams();

  /// Construct using a moose object (usually the object which inherits from the interface)
  MappingInterface(const MooseObject * moose_object);

  /**
   * Get the mapping using the parameters of the moose object
   * @param name The parameter name
   */
  VariableMappingBase & getMapping(const std::string & name) const;

  /**
   * Get the mapping by supplying the name of the object in the warehouse
   * @param name The name of the mapping object
   */
  VariableMappingBase & getMappingByName(const UserObjectName & name) const;

private:
  /// Parameters of the object with this interface
  const InputParameters & _smi_params;

  /// Reference to FEProblemBase instance
  FEProblemBase & _smi_feproblem;
};
