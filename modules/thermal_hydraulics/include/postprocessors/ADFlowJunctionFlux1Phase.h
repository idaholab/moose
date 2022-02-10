//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"

class ADFlowJunctionUserObject;

/**
 * Retrieves an entry of a flux vector for a connection attached to a 1-phase junction
 */
class ADFlowJunctionFlux1Phase : public SideIntegralPostprocessor
{
public:
  ADFlowJunctionFlux1Phase(const InputParameters & parameters);

  virtual Real computeQpIntegral() override;

protected:
  /// Index of the connected flow channel
  const unsigned int _connection_index;

  /// Junction component name
  const std::string & _junction_name;
  /// Junction user object name
  const std::string _junction_uo_name;
  /// Junction user object
  const ADFlowJunctionUserObject & _junction_uo;

  /// Index within flux vector to query
  const unsigned int _equation_index;

public:
  static InputParameters validParams();
};
