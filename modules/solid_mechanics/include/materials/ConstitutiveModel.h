#ifndef CONSTITUTIVEMODEL_H
#define CONSTITUTIVEMODEL_H

#include "Material.h"

#include "SymmElasticityTensor.h"
#include "SymmTensor.h"

/**
 */

class ConstitutiveModel : public Material
{
public:
  ConstitutiveModel( const std::string & name,
                     InputParameters parameters );

  virtual void computeStress( unsigned qp,
                              const SymmElasticityTensor & elasticityTensor,
                              const SymmTensor & strain_increment,
                              const SymmTensor & stress_old,
                              SymmTensor & inelastic_strain_increment,
                              SymmTensor & stress_new );

protected:
  virtual void computeStressInitialize(unsigned /*qp*/,
                                       Real /*effectiveTrialStress*/,
                                       const SymmElasticityTensor & /*elasticityTensor*/) {}
  virtual void computeStressFinalize(unsigned /*qp*/,
                                     const SymmTensor & /*inelasticStrainIncrement*/) {}


  virtual void iterationInitialize(unsigned /*qp*/, Real /*scalar*/) {}
  virtual Real computeResidual(unsigned qp, Real effectiveTrialStress, Real scalar) = 0;
  virtual Real computeDerivative(unsigned qp, Real effectiveTrialStress, Real scalar) = 0;
  virtual void iterationFinalize(unsigned /*qp*/, Real /*scalar*/) {}

  const unsigned int _max_its;
  const bool _output_iteration_info;
  const Real _relative_tolerance;
  const Real _absolute_tolerance;

  const bool _has_temp;
  VariableValue & _temperature;
  VariableValue & _temperature_old;

private:
  using Material::computeProperties;
  using Material::_qp;

};

template<>
InputParameters validParams<ConstitutiveModel>();

#endif // CONSTITUTIVEMODEL_H
