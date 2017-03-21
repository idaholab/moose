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

template <>
InputParameters validParams<GeneralizedPlaneStrainUserObject>();

class GeneralizedPlaneStrainUserObject : public ElementUserObject
{
public:
  GeneralizedPlaneStrainUserObject(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void threadJoin(const UserObject & uo) override;
  void finalize() override;
  virtual Real returnResidual() const;
  virtual Real returnJacobian() const;

protected:
  std::string _base_name;

  const MaterialProperty<RankFourTensor> & _Cijkl;
  const MaterialProperty<RankTwoTensor> & _stress;

  Function & _out_of_plane_pressure;
  const Real _factor;

private:
  unsigned int _scalar_out_of_plane_strain_direction;
  Real _residual;
  Real _jacobian;
};

#endif // GENERALIZEDPLANESTRAINUSEROBJECT_H
