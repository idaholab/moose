#ifndef SOLIDMECHANICSMATERIAL_H
#define SOLIDMECHANICSMATERIAL_H

#include "Material.h"


//Forward Declarations
class SolidMechanicsMaterial;
class ElasticityTensor;

template<>
InputParameters validParams<SolidMechanicsMaterial>();

/**
 * SolidMechanics material for use in simple applications that don't need material properties.
 */
class SolidMechanicsMaterial : public Material
{
public:
  SolidMechanicsMaterial(std::string name,
                         InputParameters parameters,
                         unsigned int block_id,
                         std::vector<std::string> coupled_to,
                         std::vector<std::string> coupled_as);
  
protected:
  std::vector<RealGradient> & _grad_disp_x;
  std::vector<RealGradient> & _grad_disp_y;
  std::vector<RealGradient> & _grad_disp_z;

  std::vector<RealTensorValue> & _stress;
};

#endif //SOLIDMECHANICSMATERIAL_H
