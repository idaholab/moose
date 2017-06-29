/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GENERALIZEDMAXWELLBASE_H
#define GENERALIZEDMAXWELLBASE_H

#include "LinearViscoelasticityBase.h"

class GeneralizedMaxwellBase;

template <>
InputParameters validParams<GeneralizedMaxwellBase>();

/**
 * Abstract generalized Maxwell model (m Maxwell modules
 * placed in parallel).
 *
 * This class must be inherited so that the properties of each spring and
 * dashpot is properly defined.
 *
 * If this model contains a stand-alone dashpot, then it may not converge under
 * Dirichlet boundary conditions (imposed displacements).
 */
class GeneralizedMaxwellBase : public LinearViscoelasticityBase
{
public:
  GeneralizedMaxwellBase(const InputParameters & parameters);

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

  // we only need to be warned once
  bool _has_been_warned;

private:
  void emitWarning();
};

#endif // GENERALIZEDMAXWELLBASE_H
