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

class SystemBase;

/**
 *
 */
class CreateDisplacedProblemAction : public Action
{
public:
  static InputParameters validParams();

  CreateDisplacedProblemAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /**
   * Sets up a ProxyRelationshipManager that copies algebraic ghosting from->to
   */
  void addProxyAlgebraicRelationshipManagers(SystemBase & to, SystemBase & from);

  /**
   * Sets up a ProxyRelationshipManager that copies geometric ghosting from->to
   */
  void addProxyGeometricRelationshipManagers(SystemBase & to, SystemBase & from);

private:
  /**
   * Generic adder of ProxyRelationshipManagers
   * @param to The system to add RelationshipManagers for
   * @param from The system to copy RelationshipManagers over from
   * @param rm_type The Moose::RelationshipManagerType, e.g. GEOMETRIC or ALGEBRAIC (COUPLING
   *                doesn't need to be copied back and forth)
   * @param type A string form of the type, e.g. "geometric", "algebraic", or "coupling"
   */
  void addProxyRelationshipManagers(SystemBase & to,
                                    SystemBase & from,
                                    Moose::RelationshipManagerType rm_type,
                                    std::string type);
};
