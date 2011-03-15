#ifndef SOLIDMECHANICSMATERIALRZ_H
#define SOLIDMECHANICSMATERIALRZ_H

#include "Material.h"


//Forward Declarations
class SolidMechanicsMaterialRZ;
class VolumetricModel;

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

  virtual void subdomainSetup();

  bool _initialized;

  VariableValue & _disp_r;
  VariableValue & _disp_z;

  VariableGradient & _grad_disp_r;
  VariableGradient & _grad_disp_z;

  bool _has_temp;
  VariableValue & _temp;

  std::vector<VolumetricModel*> _volumetric_models;

  MaterialProperty<RealTensorValue> & _stress;
  MaterialProperty<ColumnMajorMatrix> & _elasticity_tensor;
  MaterialProperty<ColumnMajorMatrix> & _Jacobian_mult;
  MaterialProperty<ColumnMajorMatrix> & _elastic_strain;

};

#endif //SOLIDMECHANICSMATERIALRZ_H
