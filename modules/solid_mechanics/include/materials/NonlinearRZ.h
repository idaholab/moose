/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NONLINEARRZ_H
#define NONLINEARRZ_H

#include "Nonlinear.h"

namespace SolidMechanics
{

/**
 * NonlinearRZ is the base class for all RZ nonlinear solid mechanics material models.
 */
class NonlinearRZ : public Nonlinear
{
public:
  NonlinearRZ( SolidModel & solid_model,
               const std::string & name,
               InputParameters parameters );

  virtual ~NonlinearRZ();

  VariableGradient & _grad_disp_r;
  VariableGradient & _grad_disp_z;
  VariableGradient & _grad_disp_r_old;
  VariableGradient & _grad_disp_z_old;
  VariableValue & _disp_r;
  VariableValue & _disp_r_old;

protected:

  virtual void computeDeformationGradient( unsigned int qp, ColumnMajorMatrix & F);
  virtual void fillMatrix( unsigned int qp,
                           const VariableGradient & grad_r,
                           const VariableGradient & grad_z,
                           const VariableValue & u,
                           ColumnMajorMatrix & A) const;

  virtual Real volumeRatioOld(unsigned qp) const;

  virtual void computeIncrementalDeformationGradient( std::vector<ColumnMajorMatrix> & Fhat);



};

} // namespace solid_mechanics


#endif
