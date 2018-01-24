/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PFCRFFMATERIAL_H
#define PFCRFFMATERIAL_H

#include "Material.h"

// Forward Declarations
class PFCRFFMaterial;

template <>
InputParameters validParams<PFCRFFMaterial>();

class PFCRFFMaterial : public Material
{
public:
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

#endif // PFCRFFMATERIAL_H
