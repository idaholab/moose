/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GENERALIZEDPLANESTRAINOFFDIAG_H
#define GENERALIZEDPLANESTRAINOFFDIAG_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

//Forward Declarations
class GeneralizedPlaneStrainOffDiag;
class RankTwoTensor;
class RankFourTensor;

template<>
InputParameters validParams<GeneralizedPlaneStrainOffDiag>();

class GeneralizedPlaneStrainOffDiag : public DerivativeMaterialInterface<Kernel>
{
public:
  GeneralizedPlaneStrainOffDiag(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() { return 0; }

 /**
  * These methods are used to compute the off-diagonal jacobian for the coupling
  * between scalar variable strain_zz and nonlinear variables displacements and temperature.
  * disp indicates the coupling is between displacements and strain_zz and
  * temp is for temperature and strain_zz
  */
  virtual void computeOffDiagJacobianScalar(unsigned int jvar);
  virtual void computeDispOffDiagJacobianScalar(unsigned int component, unsigned int jvar);
  virtual void computeTempOffDiagJacobianScalar(unsigned int jvar);

  std::string _base_name;

  const MaterialProperty<RankFourTensor> & _Jacobian_mult;
  const std::vector<MaterialPropertyName> _eigenstrain_names;
  std::vector<const MaterialProperty<RankTwoTensor> *> _deigenstrain_dT;

  const unsigned int _scalar_strain_zz_var;

  MooseVariable * _temp_var;

  std::vector<MooseVariable *> _disp_var;
};
#endif //GENERALIZEDPLANESSTRAINOFFDIAG_H
