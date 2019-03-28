//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEVALUEUO_QP
#define INTERFACEVALUEUO_QP

#include "InterfaceValueUserObject.h"

class InterfaceValueUO_QP;

template <>
InputParameters validParams<InterfaceValueUO_QP>();

/**
 * This userobject collect values of a variable across an interface for each QP and compute a
 * scalar. The computed scalar value depends on the given parameter _interface_value_type\
 * _interface_value_type (see IntervafeValueTools).
 */
class InterfaceValueUO_QP : public InterfaceValueUserObject
{
public:
  InterfaceValueUO_QP(const InputParameters & parameters);
  virtual ~InterfaceValueUO_QP();

  virtual void initialize();
  virtual void execute();
  virtual void finalize() { return; };
  virtual void threadJoin(const UserObject & /*uo*/) { return; };

  Real getQpValue(dof_id_type elem, unsigned int side, unsigned int qp) const;

protected:
  /// this map is used to store QP data.
  std::map<std::pair<dof_id_type, unsigned int>, std::vector<Real>> _map_values;
  const VariableValue & _u;
  const VariableValue & _u_neighbor;
};

#endif // INTERFACEVALUEUO_QP
