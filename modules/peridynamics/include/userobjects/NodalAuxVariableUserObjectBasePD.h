//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObjectBasePD.h"

/**
 * UserObject base class to compute nodal quantities stored as AuxVariable at a material point based
 * on elemental information of bonds connected at the material point
 */
class NodalAuxVariableUserObjectBasePD : public ElementUserObjectBasePD
{
public:
  static InputParameters validParams();

  NodalAuxVariableUserObjectBasePD(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & /*uo*/) override{};
  virtual void finalize() override;

protected:
  /**
   * Function to assemble elemental quantities to nodal AuxVariable at a material point
   * @param id   The local index of element node (either 1 or 2 for Edge2 element)
   * @param dof   The global DOF of element node
   * @return The computed value for the node
   */
  virtual void computeValue(unsigned int id, dof_id_type dof) = 0;

  /// Pointer to the aux variable this userobject operates on
  MooseVariable * _var;
};
