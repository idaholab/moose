/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECREEPSTRAINBASE_H
#define COMPUTECREEPSTRAINBASE_H

#include "Material.h"
#include "RankTwoTensor.h"

class ComputeCreepStrainBase;

template <>
InputParameters validParams<ComputeCreepStrainBase>();

/**
 * Base class for creep strains. Creep strains are part of the mechanical
 * strain field, but distinct from the elastic strain.
 *
 * Creep strains may be updated at the end of each time step according to
 * the current strain and stress state. For the update to occur, a ViscoelasticityUpdate
 * user-object must be defined in the simulation.
 */
class ComputeCreepStrainBase : public Material
{
public:
  ComputeCreepStrainBase(const InputParameters & parameters);

  // this method is called by ViscoelasticityUpdate at the end of each time step
  virtual void updateQpViscousStrain(unsigned int qp,
                                     const RankTwoTensor & strain,
                                     const RankTwoTensor & stress) = 0;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  // actual creep computation
  virtual void computeQpCreepStrain() = 0;

  // base name in order to differentiate creep from different origins (if necessary)
  std::string _base_name;
  std::string _creep_strain_name;

  // used for incremental models
  bool _incremental_form;

  // primary creep strain variable
  MaterialProperty<RankTwoTensor> & _creep_strain;

  // previous value of the creep strain variable (for incremental models)
  MaterialProperty<RankTwoTensor> * _creep_strain_old;

  // the creep strain must not be calculated for the 1st time step
  bool & _step_zero;
};

#endif // COMPUTECREEPSTRAINBASE_H
