//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceUserObject.h"

/**
 * This userobject tests the capabilities of the interface user object to get in-sync values of
 * bulk, boundary and interface material property. Don't use for other purposes.
 */
class InterfaceUserObjectTestGetMaterialProperty : public InterfaceUserObject
{
public:
  static InputParameters validParams();

  InterfaceUserObjectTestGetMaterialProperty(const InputParameters & parameters);
  virtual ~InterfaceUserObjectTestGetMaterialProperty();

  virtual void initialSetup() override;
  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override{};
  virtual void threadJoin(const UserObject & /*uo*/) override{};

protected:
  /// this map is used to store QP data.
  std::map<std::pair<dof_id_type, unsigned int>, std::vector<Real>> _map_values;

  /// the primary side, coupled bulk material property
  const MaterialProperty<Real> & _mp;
  /// the primary side, coupled bulk material property
  const MaterialProperty<Real> & _mp_neighbor;
  /// the coupled boundary material property
  const MaterialProperty<Real> & _mp_boundary;
  /// the coupled interface material property
  const MaterialProperty<Real> & _mp_interface;
};
