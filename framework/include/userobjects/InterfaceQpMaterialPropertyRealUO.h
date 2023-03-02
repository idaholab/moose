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

/**
 * Specialization of InterfaceQpMaterialPropertyBaseUserObject for Real material properties.
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
  /**
   * return the material property value at the give qp
   **/
  virtual Real computeScalarMaterialProperty(const MaterialProperty<Real> * p,
                                             const unsigned int qp) override final
  {
    return (*p)[qp];
  }
};
