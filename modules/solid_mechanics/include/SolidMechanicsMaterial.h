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
  SolidMechanicsMaterial(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  std::vector<RealGradient> & _grad_disp_x;
  std::vector<RealGradient> & _grad_disp_y;
  std::vector<RealGradient> & _grad_disp_z;

  std::vector<RealTensorValue> & _stress;
  std::vector<ColumnMajorMatrix> & _elasticity_tensor;
};

#endif //SOLIDMECHANICSMATERIAL_H
