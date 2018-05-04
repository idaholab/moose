/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef XFEMDIFFUSION_H
#define XFEMDIFFUSION_H

#include "Diffusion.h"
#include "Material.h"

// Forward Declarations
class XFEMDiffusion;

template <>
InputParameters validParams<XFEMDiffusion>();

class XFEMDiffusion : public Diffusion
{
public:
  XFEMDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  std::string _base_name;
  std::string _diffusion_coefficient_name;

  const MaterialProperty<Real> & _diffusion_coefficient;
  const MaterialProperty<Real> * const _diffusion_coefficient_dT;
};

#endif // XFEMDIFFUSION_H
