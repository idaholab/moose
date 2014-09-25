#ifndef CONSERVEDLANGEVINNOISE_H
#define CONSERVEDLANGEVINNOISE_H

#include "LangevinNoise.h"
#include "ConservedNoiseBase.h"

//Forward Declarations
class ConservedLangevinNoise;

template<>
InputParameters validParams<LangevinNoise>();

class ConservedLangevinNoise : public LangevinNoise
{
public:
  ConservedLangevinNoise(const std::string & name, InputParameters parameters);

protected:
  virtual void residualSetup() {};
  virtual Real computeQpResidual();

private:
  const ConservedNoiseInterface & _noise;
};

#endif //CONSERVEDLANGEVINNOISE_H
