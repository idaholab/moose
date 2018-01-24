/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPHERICALR_H
#define SPHERICALR_H

#include "Element.h"

// Forward Declarations
class SymmElasticityTensor;

namespace SolidMechanics
{

class SphericalR : public Element
{
public:
  SphericalR(SolidModel & solid_model,
             const std::string & name,
             const InputParameters & parameters);
  virtual ~SphericalR();

protected:
  virtual void computeStrain(const unsigned qp,
                             const SymmTensor & total_strain_old,
                             SymmTensor & total_strain_new,
                             SymmTensor & strain_increment);

  virtual unsigned int getNumKnownCrackDirs() const { return 2; }

  const VariableValue & _disp_r;

  const bool _large_strain;

  const VariableGradient & _grad_disp_r;
};
}

#endif
