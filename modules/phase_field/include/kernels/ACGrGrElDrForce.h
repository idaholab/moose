#ifndef ACGRGRELDRFORCE_H
#define ACGRGRELDRFORCE_H

#include "ACBulk.h"

//Forward Declarations
class ACGrGrElDrForce;
class RankTwoTensor;
class ElasticityTensorR4;

template<>
InputParameters validParams<ACGrGrElDrForce>();
/**
 * Calculates the porton of the Allen-Cahn equation that results from the deformation energy.
 * Must access the elastic_strain stored as a material property
 * Requires the name of the elastic tensor derivative as an input.
 */
class ACGrGrElDrForce : public ACBulk
{
public:
  ACGrGrElDrForce(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);

private:
  const MaterialProperty<ElasticityTensorR4> & _D_elastic_tensor;
  const MaterialProperty<RankTwoTensor> & _elastic_strain;
};

#endif //ACGRGRELDRFORCE_H
