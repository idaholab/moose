//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

// Forward Declarations

class PFCRFFMaterial : public Material
{
public:
  static InputParameters validParams();

  PFCRFFMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  MaterialProperty<Real> & _M;
  MaterialProperty<Real> & _alpha_R_0;
  MaterialProperty<Real> & _alpha_I_0;
  MaterialProperty<Real> & _A_R_0;
  MaterialProperty<Real> & _A_I_0;
  MaterialProperty<Real> & _alpha_R_1;
  MaterialProperty<Real> & _alpha_I_1;
  MaterialProperty<Real> & _A_R_1;
  MaterialProperty<Real> & _A_I_1;
  MaterialProperty<Real> & _alpha_R_2;
  MaterialProperty<Real> & _alpha_I_2;
  MaterialProperty<Real> & _A_R_2;
  MaterialProperty<Real> & _A_I_2;
  MaterialProperty<Real> & _alpha_R_3;
  MaterialProperty<Real> & _alpha_I_3;
  MaterialProperty<Real> & _A_R_3;
  MaterialProperty<Real> & _A_I_3;
  MaterialProperty<Real> & _alpha_R_4;
  MaterialProperty<Real> & _alpha_I_4;
  MaterialProperty<Real> & _A_R_4;
  MaterialProperty<Real> & _A_I_4;

  unsigned int _num_L;
};
