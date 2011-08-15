#ifndef LINEARISOTROPICMATERIALRZ_H
#define LINEARISOTROPICMATERIALRZ_H

#include "SolidMechanicsMaterialRZ.h"

//Forward Declarations
class LinearIsotropicMaterialRZ;
class ElasticityTensor;

template<>
InputParameters validParams<LinearIsotropicMaterialRZ>();

/**
 * AxisymmetriclinearIsotropic material for use in simple applications that don't need material properties.
 */
class LinearIsotropicMaterialRZ : public SolidMechanicsMaterialRZ
{
public:
  LinearIsotropicMaterialRZ(const std::string & name,
                            InputParameters parameters);

  virtual ~LinearIsotropicMaterialRZ();

protected:

  /**
   * Will always be passed to full symmetric strain tensor.
   * What should come out is a modified strain tensor.
   */
  virtual void computeNetElasticStrain(const SymmTensor & input_strain, SymmTensor & elastic_strain);

};

#endif //LINEARISOTROPICMATERIALRZ_H
