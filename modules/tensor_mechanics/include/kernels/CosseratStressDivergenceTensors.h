/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COSSERATSTRESSDIVERGENCETENSORS_H
#define COSSERATSTRESSDIVERGENCETENSORS_H

#include "StressDivergenceTensors.h"

//Forward Declarations
class CosseratStressDivergenceTensors;

template<>
InputParameters validParams<CosseratStressDivergenceTensors>();

class CosseratStressDivergenceTensors : public StressDivergenceTensors
{
public:
  CosseratStressDivergenceTensors(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const unsigned int _wc_x_var;
  const unsigned int _wc_y_var;
  const unsigned int _wc_z_var;
};

#endif //COSSERATSTRESSDIVERGENCETENSORS_H
