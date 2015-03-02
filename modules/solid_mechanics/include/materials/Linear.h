/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LINEAR_H
#define LINEAR_H

#include "Element.h"

//Forward Declarations
class SymmElasticityTensor;

namespace SolidMechanics
{

class Linear : public Element
{
public:
  Linear(SolidModel & solid_model, const std::string & name, InputParameters parameters);
  virtual ~Linear();

protected:

  virtual void computeStrain( const unsigned qp,
                              const SymmTensor & total_strain_old,
                              SymmTensor & total_strain_new,
                              SymmTensor & strain_increment );

  const bool _large_strain;

  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;
  VariableGradient & _grad_disp_z;

};

}

#endif
