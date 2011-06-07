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
                            SymmTensor & strain_increment) = 0;

};

#endif // VOLUMETRICMODEL_H
