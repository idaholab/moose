/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef Q2PSATURATIONDIFFUSION
#define Q2PSATURATIONDIFFUSION

#include "Kernel.h"
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"
#include "Material.h"
#include "Function.h"

// Forward Declarations
class Q2PSaturationDiffusion;

template <>
InputParameters validParams<Q2PSaturationDiffusion>();

/**
 * Diffusive Kernel that models nonzero capillary pressure in Q2P models
 * The Variable of this Kernel should be the saturation
 */
class Q2PSaturationDiffusion : public Kernel
{
public:
  Q2PSaturationDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// water density
  const RichardsDensity & _density;

  /// water relative permeability
  const RichardsRelPerm & _relperm;

  /// porepressure at the quadpoints
  const VariableValue & _pp;

  /// variable number of the porepressure variable
  unsigned int _pp_var_num;

  /// fluid viscosity
  Real _viscosity;

  /// permeability
  const MaterialProperty<RealTensorValue> & _permeability;

  Real _diffusivity;
};

#endif // Q2PSATURATIONDIFFUSION
