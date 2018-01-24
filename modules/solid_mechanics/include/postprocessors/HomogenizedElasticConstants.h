/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HOMOGENIZEDELASTICCONSTANTS_H
#define HOMOGENIZEDELASTICCONSTANTS_H

#include "ElementAverageValue.h"

// Forward Declarations
class HomogenizedElasticConstants;
class ColumnMajorMatrix;
class SymmElasticityTensor;
class SymmTensor;

template <>
InputParameters validParams<HomogenizedElasticConstants>();

/**
 * This postprocessor computes the average grain area in a polycrystal
*/
class HomogenizedElasticConstants : public ElementAverageValue
{
public:
  HomogenizedElasticConstants(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

protected:
  virtual Real computeQpIntegral();

private:
  const VariableGradient & _grad_disp_x_xx;
  const VariableGradient & _grad_disp_y_xx;
  const VariableGradient & _grad_disp_z_xx;

  const VariableGradient & _grad_disp_x_yy;
  const VariableGradient & _grad_disp_y_yy;
  const VariableGradient & _grad_disp_z_yy;

  const VariableGradient & _grad_disp_x_zz;
  const VariableGradient & _grad_disp_y_zz;
  const VariableGradient & _grad_disp_z_zz;

  const VariableGradient & _grad_disp_x_xy;
  const VariableGradient & _grad_disp_y_xy;
  const VariableGradient & _grad_disp_z_xy;

  const VariableGradient & _grad_disp_x_yz;
  const VariableGradient & _grad_disp_y_yz;
  const VariableGradient & _grad_disp_z_yz;

  const VariableGradient & _grad_disp_x_zx;
  const VariableGradient & _grad_disp_y_zx;
  const VariableGradient & _grad_disp_z_zx;

  const MaterialProperty<SymmElasticityTensor> & _elasticity_tensor;
  const unsigned int _column, _row;
  unsigned _I, _J;
  unsigned _l, _k;
  unsigned _i, _j;
  Real _volume;
  Real _integral_value;
};

#endif // HOMOGENIZEDELASTICCONSTANTS_H
