//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceValueUserObject.h"

class InterfaceQpUserObjectBase;

template <>
InputParameters validParams<InterfaceQpUserObjectBase>();

/**
 * This is a base class for userobjects collecting values of variables or material properites across
 * an interface at each QP. This userobejct class always return a scalar value and compute both the
 * current value or its rate. The computed scalar value depends on the given parameter
 * interface_value_type paramters (see IntervafeValueTools). Also, it provides two output options to
 * all other MOOSE systems, a qp value (getQpValue) or an element side average value
 * (getSideAverageValue).
 */
class InterfaceQpUserObjectBase : public InterfaceValueUserObject
{
public:
  static InputParameters validParams();

  InterfaceQpUserObjectBase(const InputParameters & parameters);
  virtual ~InterfaceQpUserObjectBase(){};
  virtual void initialSetup() override;
  virtual void initialize(){};
  virtual void execute();
  virtual void finalize() { return; };
  virtual void threadJoin(const UserObject & /*uo*/) { return; };

  Real getQpValue(const dof_id_type elem, const unsigned int side, unsigned int qp) const;

  Real getSideAverageValue(const dof_id_type elem, const unsigned int side) const;

protected:
  const bool _compute_rate;
  /// this map is used to store QP data.
  std::map<std::pair<dof_id_type, unsigned int>, std::vector<Real>> _map_values;
  std::map<std::pair<dof_id_type, unsigned int>, std::vector<Real>> _map_JxW;
  virtual Real computeRealValueMaster(const unsigned int /*qp*/) = 0;
  virtual Real computeRealValueSlave(const unsigned int /*qp*/) = 0;
};
