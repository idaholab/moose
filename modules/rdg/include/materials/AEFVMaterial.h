/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef AEFVMATERIAL_H
#define AEFVMATERIAL_H

#include "Material.h"
#include "SlopeLimitingBase.h"

class AEFVMaterial;

template <>
InputParameters validParams<AEFVMaterial>();

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

#endif
