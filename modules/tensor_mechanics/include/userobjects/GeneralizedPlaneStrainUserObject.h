/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GENERALIZEDPLANESTRAINUSEROBJECT_H
#define GENERALIZEDPLANESTRAINUSEROBJECT_H

#include "ElementUserObject.h"
#include "SubblockIndexProvider.h"

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
  virtual Real returnResidual(unsigned int scalar_var_id = 0) const;
  virtual Real returnJacobian(unsigned int scalar_var_id = 0) const;

protected:
  std::string _base_name;

  const MaterialProperty<RankFourTensor> & _Cijkl;
  const MaterialProperty<RankTwoTensor> & _stress;

  const SubblockIndexProvider * _subblock_id_provider;

  Function & _out_of_plane_pressure;
  const Real _factor;
  unsigned int _scalar_out_of_plane_strain_direction;
  std::vector<Real> _residual;
  std::vector<Real> _jacobian;
};

#endif // GENERALIZEDPLANESTRAINUSEROBJECT_H
