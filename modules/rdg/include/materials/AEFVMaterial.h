//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "SlopeLimitingBase.h"

/**
 * A material kernel for the advection equation
 * using a cell-centered finite volume method
 *
 * Notes:
 *
 *   1. This material kernel is responsible for invoking
 *      the in-cell slope reconstruction and slope limiting
 *      based on the cell-average variable values.
 *
 *   2. The reconstructed/limited linear polynomial variable
 *      in each element is then interpolated to the center of each side
 *      that surrounds an element.
 *
 *   3. If a system of governing equations is being solved,
 *      the reconstructed/limited vector of variable gradients
 *      is only calculated once when the first equation is dealt with,
 *      and then cached for use for the rest of the equations.
 *
 *   4. If reconstruction/limiting scheme is not turned on,
 *      the cell-average constant variable is used for flux calculations,
 *      which is first-order accurate in space and absolutely stable
 */
class AEFVMaterial : public Material
{
public:
  static InputParameters validParams();

  AEFVMaterial(const InputParameters & parameters);
  virtual ~AEFVMaterial();

protected:
  virtual void computeQpProperties();

  // cell-average variable
  const VariableValue & _uc;

  // slope limiting user objects
  const SlopeLimitingBase & _lslope;

  // derived variables at face center
  MaterialProperty<Real> & _u;
};
