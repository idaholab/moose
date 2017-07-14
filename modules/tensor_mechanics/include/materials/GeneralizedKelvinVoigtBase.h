/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GENERALIZEDKELVINVOIGTBASE_H
#define GENERALIZEDKELVINVOIGTBASE_H

#include "LinearViscoelasticityBase.h"

class GeneralizedKelvinVoigtBase;

template <>
InputParameters validParams<GeneralizedKelvinVoigtBase>();

/**
 * Abstract generalized Kelvin-Voigt model (m Kelvin-Voigt modules
 * placed in series).
 *
 * This class must be inherited so that the properties of each spring and
 * dashpot is properly defined.
 */
class GeneralizedKelvinVoigtBase : public LinearViscoelasticityBase
{
public:
  GeneralizedKelvinVoigtBase(const InputParameters & parameters);

  // transforms the viscous strains into an apparent creep strain
  virtual void
  accumulateQpViscousStrain(unsigned int qp,
                            RankTwoTensor & accumulated_viscous_strain,
                            const std::vector<RankTwoTensor> & viscous_strains,
                            bool has_driving_eigenstrain = false,
                            const RankTwoTensor & driving_eigenstrain = RankTwoTensor()) const;

  // updates the viscous strains at the end of each time step
  virtual void
  updateQpViscousStrain(unsigned int qp,
                        std::vector<RankTwoTensor> & viscous_strains,
                        const std::vector<RankTwoTensor> & viscous_strains_old,
                        const RankTwoTensor & effective_strain,
                        const RankTwoTensor & effective_stress,
                        bool has_driving_eigenstrain = false,
                        const RankTwoTensor & driving_eigenstrain = RankTwoTensor()) const;

protected:
  // transforms the springs and dashpots into an apparent and instantaneous elasticity tensors
  virtual void computeQpApparentElasticityTensors();
};

#endif // GENERALIZEDKELVINVOIGTBASE_H
