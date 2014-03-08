#ifndef VOLUMETRICMODEL_H
#define VOLUMETRICMODEL_H

#include "Material.h"

class SymmTensor;

class VolumetricModel;

template<>
InputParameters validParams<VolumetricModel>();

class VolumetricModel : public Material
{
public:
  VolumetricModel( const std::string & name,
                   InputParameters & parameters );
  virtual ~VolumetricModel();

  virtual void modifyStrain(const unsigned int qp,
                            const Real scale_factor,
                            SymmTensor & strain_increment,
                            SymmTensor & dstrain_increment_dT) = 0;
private:
  using Material::_qp;
};

#endif // VOLUMETRICMODEL_H
