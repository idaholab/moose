//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  NonlinearRZ(SolidModel & solid_model,
              const std::string & name,
              const InputParameters & parameters);

  virtual ~NonlinearRZ();

  const VariableGradient & _grad_disp_r;
  const VariableGradient & _grad_disp_z;
  const VariableGradient & _grad_disp_r_old;
  const VariableGradient & _grad_disp_z_old;
  const VariableValue & _disp_r;
  const VariableValue & _disp_r_old;

protected:
  virtual void computeDeformationGradient(unsigned int qp, ColumnMajorMatrix & F);
  virtual void fillMatrix(unsigned int qp,
                          const VariableGradient & grad_r,
                          const VariableGradient & grad_z,
                          const VariableValue & u,
                          ColumnMajorMatrix & A) const;

  virtual Real volumeRatioOld(unsigned qp) const;

  virtual void computeIncrementalDeformationGradient(std::vector<ColumnMajorMatrix> & Fhat);
  const bool _volumetric_locking_correction;
};

} // namespace solid_mechanics

#endif
