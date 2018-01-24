/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef VOLUMETRICMODEL_H
#define VOLUMETRICMODEL_H

#include "Material.h"

class SymmTensor;

class VolumetricModel;

template <>
InputParameters validParams<VolumetricModel>();

class VolumetricModel : public Material
{
public:
  VolumetricModel(const InputParameters & parameters);
  virtual ~VolumetricModel();

  virtual void modifyStrain(const unsigned int qp,
                            const Real scale_factor,
                            SymmTensor & strain_increment,
                            SymmTensor & dstrain_increment_dT) = 0;

  virtual std::vector<std::string> getDependentMaterialProperties() const
  {
    return std::vector<std::string>(1, "");
  }

private:
  using Material::_qp;
};

#endif // VOLUMETRICMODEL_H
