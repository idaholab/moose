//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceQpMaterialPropertyBaseUserObject.h"

// Forward declarations
class InterfaceQpMaterialPropertyRealUO;

template <>
InputParameters validParams<InterfaceQpMaterialPropertyRealUO>();

/**
 * This userobject works on Real material properties. It returns the interface value (see
 * IntervafeValueTools) or its rate.
 */
class InterfaceQpMaterialPropertyRealUO : public InterfaceQpMaterialPropertyBaseUserObject<Real>
{

public:
  static InputParameters validParams();
  /**
   * Class constructor
   * @param parameters The input parameters for this object
   */
  InterfaceQpMaterialPropertyRealUO(const InputParameters & parameters);
  virtual ~InterfaceQpMaterialPropertyRealUO(){};

protected:
  virtual Real computeRealValueMaster(const unsigned int /*qp*/) override;
  virtual Real computeRealValueSlave(const unsigned int /*qp*/) override;
};
