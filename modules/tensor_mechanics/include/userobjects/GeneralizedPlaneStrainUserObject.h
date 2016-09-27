/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GENERALIZEDPLANESTRAINUSEROBJECT_H
#define GENERALIZEDPLANESTRAINUSEROBJECT_H

#include "ElementUserObject.h"

class GeneralizedPlaneStrainUserObject;
class RankTwoTensor;
class RankFourTensor;
class Function;

template<>
InputParameters validParams<GeneralizedPlaneStrainUserObject>();

class GeneralizedPlaneStrainUserObject : public ElementUserObject
{
public:
  GeneralizedPlaneStrainUserObject(const InputParameters & parameters);
  virtual ~GeneralizedPlaneStrainUserObject() {}

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & uo){};
  virtual void finalize(){};
  virtual Real returnResidual() const;
  virtual Real returnJacobian() const;

protected:
  const MaterialProperty<RankFourTensor> & _Cijkl;
  const MaterialProperty<RankTwoTensor> & _stress;

  Function & _traction;
  const Real _factor;

private:
  Real _residual;
  Real _jacobian;
};

#endif // GENERALIZEDPLANESTRAINUSEROBJECT_H
