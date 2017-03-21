/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVMATERIAL_H
#define CNSFVMATERIAL_H

#include "Material.h"
#include "SinglePhaseFluidProperties.h"
#include "SlopeLimitingBase.h"

class CNSFVMaterial;

template <>
InputParameters validParams<CNSFVMaterial>();

/**
 * A material kernel for the CNS equations
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
class CNSFVMaterial : public Material
{
public:
  CNSFVMaterial(const InputParameters & parameters);
  virtual ~CNSFVMaterial();

protected:
  virtual void computeQpProperties();

  // piecewise constant conserved variables

  /// density
  const VariableValue & _rhoc;
  /// x-component of momentum
  const VariableValue & _rhouc;
  /// y-component of momentum
  const VariableValue & _rhovc;
  /// z-component pf momentum
  const VariableValue & _rhowc;
  /// total energy
  const VariableValue & _rhoec;

  // user objects

  /// slope limiting
  const SlopeLimitingBase & _lslope;
  /// fluid properties
  const SinglePhaseFluidProperties & _fp;

  // derived variables at face center

  /// rho
  MaterialProperty<Real> & _rho;
  /// rhou
  MaterialProperty<Real> & _rhou;
  /// rhov
  MaterialProperty<Real> & _rhov;
  /// rhow
  MaterialProperty<Real> & _rhow;
  /// rhoe
  MaterialProperty<Real> & _rhoe;
  /// velocity magnitude
  MaterialProperty<Real> & _vmag;
  /// pressure
  MaterialProperty<Real> & _pres;
  /// temperature
  MaterialProperty<Real> & _temp;
  /// specific total enthalpy
  MaterialProperty<Real> & _enth;
  /// speed of sound
  MaterialProperty<Real> & _csou;
  /// Mach number
  MaterialProperty<Real> & _mach;
  /// x-velocity
  MaterialProperty<Real> & _uadv;
  /// y-velocity
  MaterialProperty<Real> & _vadv;
  /// z-velocity
  MaterialProperty<Real> & _wadv;
};

#endif
