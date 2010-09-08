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
  SolidMechanicsMaterial(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;
  VariableGradient & _grad_disp_z;

  bool _has_temp;
  VariableValue & _temp;

  MaterialProperty<RealTensorValue> & _stress;
  MaterialProperty<ColumnMajorMatrix> & _elasticity_tensor;
  MaterialProperty<ColumnMajorMatrix> & _elastic_strain;

  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _density;
  MaterialProperty<Real> & _specific_heat;
};

#endif //SOLIDMECHANICSMATERIAL_H
