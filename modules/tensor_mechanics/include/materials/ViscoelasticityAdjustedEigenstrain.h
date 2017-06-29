/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef VISCOELASTICITYADJUSTEDEIGENSTRAIN_H
#define VISCOELASTICITYADJUSTEDEIGENSTRAIN_H

#include "ComputeLinearViscoelasticCreepStrain.h"

class ViscoelasticityAdjustedEigenstrain;

template <>
InputParameters validParams<ViscoelasticityAdjustedEigenstrain>();

/**
 * Adjusts an eigenstrain to compensate for internal viscoelastic processes.
 * The resulting "creep" strain is not a true mechanical creep strain but instead
 * an eigenstrain, and should be accounted for as an eigenstrain in the strain-stress
 * relation (in fact, it should replace the source eigenstrain in that relation).
 * This class should therefore be used in addition to another creep strain class.
 */
class ViscoelasticityAdjustedEigenstrain : public ComputeLinearViscoelasticCreepStrain
{
public:
  ViscoelasticityAdjustedEigenstrain(const InputParameters & parameters);

  // updates the internal viscous strains using the value of the source eigenstrain
  virtual void updateQpViscousStrain(unsigned int qp,
                                     const RankTwoTensor & strain,
                                     const RankTwoTensor & stress);

protected:
  // adjusts the source eigenstrain by the current value of the internal viscous strains
  virtual void computeQpCreepStrain();

  std::string _source_eigenstrain_name;
  const MaterialProperty<RankTwoTensor> & _source_eigenstrain;
};

#endif // VISCOELASTICITYADJUSTEDEIGENSTRAIN_H
