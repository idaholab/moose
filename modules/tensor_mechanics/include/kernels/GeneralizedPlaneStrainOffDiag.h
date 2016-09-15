/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GENERALIZEDPLANESTRAINOFFDIAG_H
#define GENERALIZEDPLANESTRAINOFFDIAG_H

#include "Kernel.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

//Forward Declarations
class GeneralizedPlaneStrainOffDiag;

template<>
InputParameters validParams<GeneralizedPlaneStrainOffDiag>();

class GeneralizedPlaneStrainOffDiag : public Kernel
{
public:
  GeneralizedPlaneStrainOffDiag(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(){return 0;}

  virtual void computeOffDiagJacobianScalar(unsigned int jvar);

  virtual void computeDispOffDiagJacobianScalar(unsigned int component, unsigned int jvar);

  virtual void computeTempOffDiagJacobianScalar(unsigned int jvar);

  std::string _base_name;

  const MaterialProperty<RankFourTensor> & _Jacobian_mult;
  const MaterialProperty<RankTwoTensor> & _dstress_dT;

private:
  const unsigned int _strain_zz_var;

  MooseVariable * _temp_var;
  std::vector<MooseVariable *> _disp_var;
};
#endif //GENERALIZEDPLANESSTRAINOFFDIAG_H
