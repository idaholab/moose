#ifndef RANKTWOAUX_H
#define RANKTWOAUX_H

#include "AuxKernel.h"
#include "RankTwoTensor.h"

class RankTwoAux;

/**
 * RankTwoAux is designed to take the data in the RankTwoTensor material
 * property, for example stress or strain, and output the value for the
 * supplied indices.
 */

template<>
InputParameters validParams<RankTwoAux>();

class RankTwoAux : public AuxKernel
{
public:
  RankTwoAux(const std::string & name, InputParameters parameters);

  virtual ~ RankTwoAux() {}

protected:
  virtual Real computeValue();

private:

  MaterialProperty<RankTwoTensor> & _tensor;
  const int _i;
  const int _j;
};

#endif //RANKTWOAUX_H
