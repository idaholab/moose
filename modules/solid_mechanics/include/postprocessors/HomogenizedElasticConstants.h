/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HOMOGENIZEDELASTICCONSTANTS_H
#define HOMOGENIZEDELASTICCONSTANTS_H

#include "ElementAverageValue.h"

//Forward Declarations
class HomogenizedElasticConstants;
class ColumnMajorMatrix;
class SymmElasticityTensor;
class SymmTensor;

template<>
InputParameters validParams<HomogenizedElasticConstants>();

/**
 * This postprocessor computes the average grain area in a polycrystal
*/
class HomogenizedElasticConstants : public ElementAverageValue
{
public:
  HomogenizedElasticConstants(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

protected:
  virtual Real computeQpIntegral();

private:
  VariableGradient & _grad_disp_x_xx;
  VariableGradient & _grad_disp_y_xx;
  VariableGradient & _grad_disp_z_xx;

  VariableGradient & _grad_disp_x_yy;
  VariableGradient & _grad_disp_y_yy;
  VariableGradient & _grad_disp_z_yy;

  VariableGradient & _grad_disp_x_zz;
  VariableGradient & _grad_disp_y_zz;
  VariableGradient & _grad_disp_z_zz;

  VariableGradient & _grad_disp_x_xy;
  VariableGradient & _grad_disp_y_xy;
  VariableGradient & _grad_disp_z_xy;

  VariableGradient & _grad_disp_x_yz;
  VariableGradient & _grad_disp_y_yz;
  VariableGradient & _grad_disp_z_yz;

  VariableGradient & _grad_disp_x_zx;
  VariableGradient & _grad_disp_y_zx;
  VariableGradient & _grad_disp_z_zx;

  const MaterialProperty<SymmElasticityTensor> & _elasticity_tensor;
  const unsigned int _column, _row;
  unsigned _I, _J;
  unsigned _l, _k;
  unsigned _i, _j;
  Real _volume;
  Real _integral_value;
};

#endif //HOMOGENIZEDELASTICCONSTANTS_H
