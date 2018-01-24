/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PLANESTRAIN_H
#define PLANESTRAIN_H

#include "Element.h"
#include "ScalarCoupleable.h"

namespace SolidMechanics
{

class PlaneStrain : public Element, public ScalarCoupleable
{
public:
  PlaneStrain(SolidModel & solid_model,
              const std::string & name,
              const InputParameters & parameters);
  virtual ~PlaneStrain();

protected:
  virtual void computeStrain(const unsigned qp,
                             const SymmTensor & total_strain_old,
                             SymmTensor & total_strain_new,
                             SymmTensor & strain_increment);

  virtual void computeDeformationGradient(unsigned int qp, ColumnMajorMatrix & F);

  virtual unsigned int getNumKnownCrackDirs() const { return 1; }

  const bool _large_strain;

  const VariableGradient & _grad_disp_x;
  const VariableGradient & _grad_disp_y;
  bool _have_strain_zz;
  const VariableValue & _strain_zz;
  bool _have_scalar_strain_zz;
  const VariableValue & _scalar_strain_zz;
  const bool _volumetric_locking_correction;
};
}

#endif // SOLIDMECHANICSMATERIALRZ_H
