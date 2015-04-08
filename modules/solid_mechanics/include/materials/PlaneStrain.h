/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PLANESTRAIN_H
#define PLANESTRAIN_H

#include "Element.h"

namespace SolidMechanics
{

class PlaneStrain : public Element
{
public:
  PlaneStrain(SolidModel & solid_model, const std::string & name, InputParameters parameters);
  virtual ~PlaneStrain();

protected:

  virtual void computeStrain( const unsigned qp,
                              const SymmTensor & total_strain_old,
                              SymmTensor & total_strain_new,
                              SymmTensor & strain_increment );

  virtual void computeDeformationGradient( unsigned int qp,
                                           ColumnMajorMatrix & F);

  virtual unsigned int getNumKnownCrackDirs() const
  {
    return 1;
  }

  const bool _large_strain;

  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;

};

}

#endif //SOLIDMECHANICSMATERIALRZ_H
