#ifndef FLOWREGIMESIMPLEMATERIAL_H
#define FLOWREGIMESIMPLEMATERIAL_H

#include "FlowRegimeBaseMaterial.h"

class FlowRegimeSimpleMaterial;

template <>
InputParameters validParams<FlowRegimeSimpleMaterial>();

/**
 * Computes the flow regime map for simple closures
 *
 * This class does nothing, becuase there are no flow regimes
 */
class FlowRegimeSimpleMaterial : public FlowRegimeBaseMaterial
{
public:
  FlowRegimeSimpleMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
};

#endif /* FLOWREGIMESIMPLEMATERIAL_H */
