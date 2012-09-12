#ifndef RANKTWOAUXTONKS_H
#define RANKTWOAUXTONKS_H

#include "AuxKernel.h"
#include "RankTwoTensorTonks.h"

class RankTwoTonksAux;

/**
 * RankTwoTonksAux is designed to take the data in the RankTwoTensorTonks material
 * property, for example stress or strain, and output the value for the
 * supplied indices.
 */

template<>
InputParameters validParams<RankTwoTonksAux>();

class RankTwoTonksAux : public AuxKernel
{
public:
  RankTwoTonksAux(const std::string & name, InputParameters parameters);

  virtual ~ RankTwoTonksAux() {}
  
protected:
  virtual Real computeValue();

private:

  MaterialProperty<RankTwoTensorTonks> & _tensor;
  const int _i;
  const int _j;
};

#endif //RANKTWOAUXTONKS_H
