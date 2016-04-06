/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MOMENTBALANCING_H
#define MOMENTBALANCING_H

#include "Kernel.h"

//Forward Declarations
class MomentBalancing;
class RankFourTensor;
class RankTwoTensor;

template<>
InputParameters validParams<MomentBalancing>();

class MomentBalancing : public Kernel
{
public:
  MomentBalancing(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankFourTensor> & _Jacobian_mult;

private:
  const unsigned int _component;

  const unsigned int _wc_x_var;
  const unsigned int _wc_y_var;
  const unsigned int _wc_z_var;
  const unsigned int _xdisp_var;
  const unsigned int _ydisp_var;
  const unsigned int _zdisp_var;
};

#endif //MOMENTBALANCING_H
