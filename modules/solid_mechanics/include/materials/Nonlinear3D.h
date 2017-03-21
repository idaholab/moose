/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NONLINEAR3D_H
#define NONLINEAR3D_H

#include "Nonlinear.h"

// Forward declarations
class MaterialModel;
class VolumetricModel;

namespace SolidMechanics
{

/**
 * Nonlinear3D is the base class for all 3D nonlinear solid mechanics material models.
 */
class Nonlinear3D : public Nonlinear
{
public:
  Nonlinear3D(SolidModel & solid_model,
              const std::string & name,
              const InputParameters & parameters);

  virtual ~Nonlinear3D();

protected:
  const VariableGradient & _grad_disp_x;
  const VariableGradient & _grad_disp_y;
  const VariableGradient & _grad_disp_z;
  const VariableGradient & _grad_disp_x_old;
  const VariableGradient & _grad_disp_y_old;
  const VariableGradient & _grad_disp_z_old;

  virtual void computeDeformationGradient(unsigned int qp, ColumnMajorMatrix & F);

  virtual Real volumeRatioOld(unsigned qp) const;

  virtual void computeIncrementalDeformationGradient(std::vector<ColumnMajorMatrix> & Fhat);
  const bool _volumetric_locking_correction;
};

} // namespace solid_mechanics

#endif
