#ifndef SOLIDMECHANICSMATERIAL_H
#define SOLIDMECHANICSMATERIAL_H

#include "Material.h"

//Forward Declarations
class SolidMechanicsMaterial;
class VolumetricModel;

template<>
InputParameters validParams<SolidMechanicsMaterial>();

/**
 * SolidMechanics material for use in simple applications that don't need material properties.
 */
class SolidMechanicsMaterial : public Material
{
public:
  SolidMechanicsMaterial(const std::string & name, InputParameters parameters);

protected:
  virtual void initialSetup();

  VariableGradient & _grad_disp_x;
  VariableGradient & _grad_disp_y;
  VariableGradient & _grad_disp_z;

  bool _has_temp;
  VariableValue & _temp;

  std::vector<VolumetricModel*> _volumetric_models;

  MaterialProperty<RealTensorValue> & _stress;
  MaterialProperty<ColumnMajorMatrix> & _elasticity_tensor;
  MaterialProperty<ColumnMajorMatrix> & _Jacobian_mult;

  // Accumulate derivatives of strain tensors with respect to Temperature into this
  ColumnMajorMatrix _d_strain_dT;

  // The derivative of the stress with respect to Temperature
  MaterialProperty<RealTensorValue> & _d_stress_dT;

  MaterialProperty<ColumnMajorMatrix> & _elastic_strain;

  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _density;
  MaterialProperty<Real> & _specific_heat;
};

#endif //SOLIDMECHANICSMATERIAL_H
