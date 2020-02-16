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

/**
 * This is a base class for userobjects collecting values of variables or material properites across
 * an interface at each QP. This userobejct class always return a scalar value and can compute the
 * current value or current rate or the current increment. The computed scalar depends on the
 * given interface_value_type paramters (see IntervafeValueTools). Also, it provides two
 * output options to all other MOOSE systems as a qp value (getQpValue) or an element side average
 * value (getSideAverageValue).
 */
class InterfaceQpUserObjectBase : public InterfaceValueUserObject
{
public:
  static InputParameters validParams();
  /**
   * the method defining the returning value type: value, rate or increment
   **/
  static MooseEnum valueOptions() { return MooseEnum("value rate increment", "value"); }

  InterfaceQpUserObjectBase(const InputParameters & parameters);
  virtual ~InterfaceQpUserObjectBase(){};
  virtual void initialSetup() override;
  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override{};
  virtual void threadJoin(const UserObject & /*uo*/) override{};

  /**
   * method returning the quadrature point value
   **/
  Real getQpValue(const dof_id_type elem, const unsigned int side, unsigned int qp) const;
  /**
   * function returning the element side average value
   **/
  Real getSideAverageValue(const dof_id_type elem, const unsigned int side) const;

protected:
  /**
   * moose enum deciding this userobject reture value type
   **/
  const MooseEnum _value_type;

  /**
   * method to overrid in child classes returnig a real value
   **/
  virtual Real computeRealValue(const unsigned int /*qp*/) = 0;

  /// these maps are used to store QP data.
  ///@{
  std::map<std::pair<dof_id_type, unsigned int>, std::vector<Real>> _map_values;
  std::map<std::pair<dof_id_type, unsigned int>, std::vector<Real>> _map_JxW;
  ///@}
};
