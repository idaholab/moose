/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWDARCYVELOCITYCOMPONENTLOWERDIMENSIONAL_H
#define POROUSFLOWDARCYVELOCITYCOMPONENTLOWERDIMENSIONAL_H

#include "PorousFlowDarcyVelocityComponent.h"

// Forward Declarations
class PorousFlowDarcyVelocityComponentLowerDimensional;

template <>
InputParameters validParams<PorousFlowDarcyVelocityComponentLowerDimensional>();

/**
 * Computes a component of the Darcy velocity:
 * -k_ij * krel /mu (nabla_j P - w_j)
 * where k_ij is the permeability tensor,
 * krel is the relative permeaility,
 * mu is the fluid viscosity,
 * P is the fluid pressure
 * and w_j is the fluid weight tensor that is projected in the tangent direction of this element
 * This is measured in m^3 . s^-1 . m^-2
 */
class PorousFlowDarcyVelocityComponentLowerDimensional : public PorousFlowDarcyVelocityComponent
{
public:
  PorousFlowDarcyVelocityComponentLowerDimensional(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /**
   * _tang_xi[i][qp] = dX/d(xi) = element's tangent vector in xi direction
   * where X = (x, y, z) are the spatial coordinates, and
   * i=0,...,LIBMESH_DIM is the dimensionality of the current element, and
   * qp is the quadpoint, and
   * xi is the element's isoparametric coordinate
   * Only _tang_xi[_current_elem->dim()] is used
   */
  std::vector<const std::vector<RealGradient> *> _tang_xi;

  /**
   * _tang_eta[i][qp] = dX/d(eta) = element's tangent vector in eta direction
   * where X = (x, y, z) are the spatial coordinates, and
   * i=0,...,LIBMESH_DIM is the dimensionality of the current element, and
   * qp is the quadpoint, and
   * eta is the element's isoparametric coordinate
   * Only _tang_eta[_current_elem->dim()] is used
   */
  std::vector<const std::vector<RealGradient> *> _tang_eta;
};

#endif // POROUSFLOWDARCYVELOCITYCOMPONENTLOWERDIMENSIONAL_H
