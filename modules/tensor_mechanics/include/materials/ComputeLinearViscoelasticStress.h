/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTELINEARVISCOELASTICSTRESS_H
#define COMPUTELINEARVISCOELASTICSTRESS_H

#include "ComputeLinearElasticStress.h"

class ComputeLinearViscoelasticStress;

template <>
InputParameters validParams<ComputeLinearViscoelasticStress>();

/**
 * Computes the stress using the elastic-only part of the mechanical strain field,
 * using the strain provided as the creep strain field.
 */
class ComputeLinearViscoelasticStress : public ComputeLinearElasticStress
{
public:
  ComputeLinearViscoelasticStress(const InputParameters & parameters);

protected:
  virtual void computeQpStress() override;

  std::string _creep_strain_name;
  const MaterialProperty<RankTwoTensor> & _creep_strain;
};

#endif // COMPUTELINEARVISCOELASTICSTRESS_H
