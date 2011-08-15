#ifndef SOLIDMECHANICSMATERIALRZ_H
#define SOLIDMECHANICSMATERIALRZ_H

#include "SolidModel.h"

//Forward Declarations
class SolidMechanicsMaterialRZ;
class SymmElasticityTensor;
class VolumetricModel;

template<>
InputParameters validParams<SolidMechanicsMaterialRZ>();

/**
 * AxisymmetricSolidMechanics material for use in simple applications that don't need material properties.
 */
class SolidMechanicsMaterialRZ : public SolidModel
{
public:
  SolidMechanicsMaterialRZ(const std::string & name, InputParameters parameters);
  virtual ~SolidMechanicsMaterialRZ();

protected:

  virtual void computeStrain();

  virtual void computeStress();

  virtual void computeNetElasticStrain(const SymmTensor & input_strain, SymmTensor & elastic_strain) = 0;


  VariableValue & _disp_r;
  VariableValue & _disp_z;

  const bool _large_strain;

  VariableGradient & _grad_disp_r;
  VariableGradient & _grad_disp_z;

};

#endif //SOLIDMECHANICSMATERIALRZ_H
