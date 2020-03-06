// Considers cleavage plane anisotropy in the crack propagation

#pragma once

#include "ACInterface.h"
#include "DerivativeMaterialInterface.h"
#include "JvarMapInterface.h"
#include "Kernel.h"

class ACInterfaceBetaPenalty;

template <>
InputParameters validParams<ACInterfaceBetaPenalty>();

class ACInterfaceBetaPenalty : public ACInterface
{
public:
  ACInterfaceBetaPenalty(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// term with beta penalty
  Real betaNablaPsi();

  // penalty for damage on planes not normal to the weak (favoured) cleavage
  // plane (Clayton & Knap, 2015)
  const Real _beta_penalty;

  // Plane normal to the weak cleavage plane: M in (Clayton & Knap, 2015)
  const std::vector<Real> _cleavage_plane_normal;
};
