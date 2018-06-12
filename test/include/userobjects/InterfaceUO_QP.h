//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEUO_QP_H
#define INTERFACEUO_QP_H

#include "InterfaceUserObject.h"

class InterfaceUO_QP;

template <>
InputParameters validParams<InterfaceUO_QP>();

/**
 *
 */
class InterfaceUO_QP : public InterfaceUserObject
{
public:
  InterfaceUO_QP(const InputParameters & parameters);
  virtual ~InterfaceUO_QP();

  virtual void initialize();
  virtual void execute();
  virtual void finalize() { return; };
  virtual void threadJoin(const UserObject & /*uo*/) { return; };

  Real getMeanMatProp(dof_id_type elem, unsigned int side, unsigned int qp) const;
  Real getVarJump(dof_id_type elem, unsigned int side, unsigned int qp) const;
  Real getNewBoundaryPropertyValue(dof_id_type elem, unsigned int side, unsigned int qp) const;

protected:
  /// this map is used for storing data at QPs.
  /// keys<element_id, side_id>, values<vector (1 elem_per_QP)<vector<Real (_mean_mat_prop, _var_jump)>>>
  std::map<std::pair<dof_id_type, unsigned int>, std::vector<std::vector<Real>>> _map_values;
  const VariableValue & _u;
  const VariableValue & _u_neighbor;

  const MaterialProperty<Real> & _diffusivity_prop;
  const MaterialProperty<Real> & _neighbor_diffusivity_prop;
};

#endif // INTERFACEUO_QP_H
