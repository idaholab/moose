/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GENERALIZEDPLANESTRAINUO_H
#define GENERALIZEDPLANESTRAINUO_H

#include "ElementUserObject.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "Function.h"

class GeneralizedPlaneStrainUO;

template<>
InputParameters validParams<GeneralizedPlaneStrainUO>();

class GeneralizedPlaneStrainUO : public ElementUserObject
{
public:
  GeneralizedPlaneStrainUO(const InputParameters & parameters);
  virtual ~GeneralizedPlaneStrainUO() {}

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & uo){};
  virtual void finalize(){};
  virtual Real returnResidual() const;
  virtual Real returnJacobian() const;

protected:
  const MaterialProperty<RankFourTensor> & _Cijkl;
  const MaterialProperty<RankTwoTensor> & _stress;

  Function & _function;
  const Real _factor;

private:
  Real _residual;
  Real _jacobian;
};

#endif // GENERALIZEDPLANESTRAINUO_H
