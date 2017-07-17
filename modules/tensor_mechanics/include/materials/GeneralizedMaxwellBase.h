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

#ifndef GENERALIZEDMAXWELLBASE_H
#define GENERALIZEDMAXWELLBASE_H

#include "LinearViscoelasticityBase.h"

class GeneralizedMaxwellBase;

template<>
InputParameters validParams<GeneralizedMaxwellBase>();

class GeneralizedMaxwellBase : public LinearViscoelasticityBase
{
public:
  GeneralizedMaxwellBase(const InputParameters & parameters);

  virtual void
  updateQpApparentProperties(unsigned int qp,
                             const RankTwoTensor & effective_strain,
                             const RankTwoTensor & effective_stress) final;

protected:
  virtual void computeQpApparentElasticityTensors() final;
  virtual void computeQpApparentCreepStrain() final;

};

#endif // GENERALIZEDMAXWELLBASE_H
