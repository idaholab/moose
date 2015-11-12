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

// Forward Declarations
class Q2PSaturationDiffusion;

template<>
InputParameters validParams<Q2PSaturationDiffusion>();

/**
 * Diffusive Kernel that models nonzero capillary pressure in Q2P models
 * The Variable of this Kernel should be the saturation
 * The capillary pressure is a quadratic function of the saturation Variable:
 * Pc = T(1-S)^2
 */
class Q2PSaturationDiffusion : public Kernel
{
public:

  Q2PSaturationDiffusion(const InputParameters & parameters);


protected:

  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// fluid density
  const RichardsDensity & _density;

  /// porepressure at the quadpoints
  VariableValue & _pp;

  /// variable number of the porepressure variable
  unsigned int _pp_var_num;

  /// fluid relative permeability
  const RichardsRelPerm & _relperm;

  /// fluid viscosity
  Real _viscosity;

  /// permeability
  const MaterialProperty<RealTensorValue> & _permeability;

  /// tension limit
  Real _ten_limit;


};

#endif //Q2PSATURATIONDIFFUSION
