/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef AXISYMMETRICRZ_H
#define AXISYMMETRICRZ_H

#include "Element.h"

// Forward Declarations
class SymmElasticityTensor;
namespace SolidMechanics
{

class AxisymmetricRZ : public Element
{
public:
  AxisymmetricRZ(SolidModel & solid_model,
                 const std::string & name,
                 const InputParameters & parameters);
  virtual ~AxisymmetricRZ();

protected:
  virtual void computeStrain(const unsigned qp,
                             const SymmTensor & total_strain_old,
                             SymmTensor & total_strain_new,
                             SymmTensor & strain_increment);

  virtual unsigned int getNumKnownCrackDirs() const { return 1; }

  const VariableValue & _disp_r;
  const VariableValue & _disp_z;

  const bool _large_strain;

  const VariableGradient & _grad_disp_r;
  const VariableGradient & _grad_disp_z;
  const bool _volumetric_locking_correction;
};
}

#endif // SOLIDMECHANICSMATERIALRZ_H
