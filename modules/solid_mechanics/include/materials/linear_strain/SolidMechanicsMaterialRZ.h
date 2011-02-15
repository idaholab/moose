#ifndef SOLIDMECHANICSMATERIALRZ_H
#define SOLIDMECHANICSMATERIALRZ_H

#include "Material.h"


//Forward Declarations
class SolidMechanicsMaterialRZ;
class ElasticityTensor;

template<>
InputParameters validParams<SolidMechanicsMaterialRZ>();

/**
 * AxisymmetricSolidMechanics material for use in simple applications that don't need material properties.
 */
class SolidMechanicsMaterialRZ : public Material
{
public:
  SolidMechanicsMaterialRZ(const std::string & name, InputParameters parameters);

protected:
  VariableValue & _disp_r;
  VariableValue & _disp_z;

  VariableGradient & _grad_disp_r;
  VariableGradient & _grad_disp_z;

  bool _has_temp;
  VariableValue & _temp;

  MaterialProperty<RealTensorValue> & _stress;
  MaterialProperty<ColumnMajorMatrix> & _elasticity_tensor;
  MaterialProperty<ColumnMajorMatrix> & _Jacobian_mult;
  MaterialProperty<ColumnMajorMatrix> & _elastic_strain;

};

#endif //SOLIDMECHANICSMATERIALRZ_H
