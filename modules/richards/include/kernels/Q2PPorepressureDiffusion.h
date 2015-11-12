/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef Q2PPOREPRESSUREDIFFUSION
#define Q2PPOREPRESSUREDIFFUSION

#include "Kernel.h"
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"
#include "Material.h"

// Forward Declarations
class Q2PPorepressureDiffusion;

template<>
InputParameters validParams<Q2PPorepressureDiffusion>();

/**
 * Diffusive Kernel that models nonzero capillary pressure in Q2P models
 * The Variable of this Kernel should be the Porepressure
 * The capillary pressure is a quadratic function of the saturation Variable:
 * Pc = T(1-S)^2
 */
class Q2PPorepressureDiffusion : public Kernel
{
public:

  Q2PPorepressureDiffusion(const InputParameters & parameters);


protected:

  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// fluid density
  const RichardsDensity & _density;

  /// saturation at the quadpoints
  VariableValue & _sat;

  /// gradient of saturation at the quadpoints
  VariableGradient & _grad_sat;

  /// variable number of the saturation variable
  unsigned int _sat_var_num;

  /// fluid relative permeability
  const RichardsRelPerm & _relperm;

  /// fluid viscosity
  Real _viscosity;

  /// permeability
  const MaterialProperty<RealTensorValue> & _permeability;

  /// tension limit
  Real _ten_limit;


};

#endif //Q2PPOREPRESSUREDIFFUSION
