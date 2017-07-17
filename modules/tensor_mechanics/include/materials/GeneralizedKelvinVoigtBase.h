/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/*                        Grizzly                               */
/*                                                              */
/*           (c) 2015 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef GENERALIZEDKELVINVOIGTBASE_H
#define GENERALIZEDKELVINVOIGTBASE_H

#include "LinearViscoelasticityBase.h"

class GeneralizedKelvinVoigtBase;

template<>
InputParameters validParams<GeneralizedKelvinVoigtBase>();

class GeneralizedKelvinVoigtBase : public LinearViscoelasticityBase
{
public:
  GeneralizedKelvinVoigtBase(const InputParameters & parameters);

  virtual void
  updateQpApparentProperties(unsigned int qp,
                             const RankTwoTensor & effective_strain,
                             const RankTwoTensor & effective_stress) final;

protected:
  virtual void computeQpApparentElasticityTensors() final;
  virtual void computeQpApparentCreepStrain() final;

};

#endif // GENERALIZEDKELVINVOIGTBASE_H
