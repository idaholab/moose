//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALAUXVARIABLEUSEROBJECTBASEPD_H
#define NODALAUXVARIABLEUSEROBJECTBASEPD_H

#include "ElementUserObjectBasePD.h"

class NodalAuxVariableUserObjectBasePD;

template <>
InputParameters validParams<NodalAuxVariableUserObjectBasePD>();

/**
 * UserObject base class to compute nodal quantities stored as AuxVariable at a material point based
 * on elemental information of bonds connected at the material point
 */
class NodalAuxVariableUserObjectBasePD : public ElementUserObjectBasePD
{
public:
  NodalAuxVariableUserObjectBasePD(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

protected:
  /**
   * Function to assemble elemental quantities to nodal AuxVariable at a material point
   */
  virtual void computeValue(unsigned int id, dof_id_type dof) = 0;

  /// Aux variable this userobject operates on
  MooseVariable * _aux_var;
};

#endif // NODALAUXVARIABLEUSEROBJECTBASEPD_H
