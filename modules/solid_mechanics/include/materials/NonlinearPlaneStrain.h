/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NONLINEARPLANESTRAIN_H
#define NONLINEARPLANESTRAIN_H

#include "Nonlinear.h"
#include "ScalarCoupleable.h"

namespace SolidMechanics
{

/**
 * NonlinearPlaneStrain is a class for large deformation plane strain
 */
class NonlinearPlaneStrain :
  public Nonlinear,
  public ScalarCoupleable
{
public:
  NonlinearPlaneStrain( SolidModel & solid_model,
                        const std::string & name,
                        const InputParameters & parameters );

  virtual ~NonlinearPlaneStrain();

  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;
  bool _have_strain_zz;
  VariableValue & _strain_zz;
  bool _have_scalar_strain_zz;
  VariableValue & _scalar_strain_zz;

  VariableGradient & _grad_disp_x_old;
  VariableGradient & _grad_disp_y_old;
  VariableValue & _strain_zz_old;
  VariableValue & _scalar_strain_zz_old;

protected:

  virtual void computeDeformationGradient( unsigned int qp, ColumnMajorMatrix & F);
  virtual void fillMatrix( unsigned int qp,
                           const VariableGradient & grad_x,
                           const VariableGradient & grad_y,
                           const Real & strain_zz,
                           ColumnMajorMatrix & A) const;

  virtual Real volumeRatioOld(unsigned qp) const;

  virtual void computeIncrementalDeformationGradient( std::vector<ColumnMajorMatrix> & Fhat);

};

} // namespace solid_mechanics


#endif
