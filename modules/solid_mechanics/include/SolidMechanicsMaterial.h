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
  MooseArray<RealGradient> & _grad_disp_x;
  MooseArray<RealGradient> & _grad_disp_y;
  MooseArray<RealGradient> & _grad_disp_z;

  MooseArray<RealTensorValue> & _stress;
  MooseArray<ColumnMajorMatrix> & _elasticity_tensor;
};

#endif //SOLIDMECHANICSMATERIAL_H
