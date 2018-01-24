//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
