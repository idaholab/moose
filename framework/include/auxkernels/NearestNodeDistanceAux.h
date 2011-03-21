#ifndef NEARESTNODEDISTANCEAUX_H
#define NEARESTNODEDISTANCEAUX_H

#include "AuxKernel.h"
#include "NearestNodeLocator.h"


//Forward Declarations
class NearestNodeDistanceAux;

template<>
InputParameters validParams<NearestNodeDistanceAux>();

/**
 * Constant auxiliary value
 */
class NearestNodeDistanceAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NearestNodeDistanceAux(const std::string & name, InputParameters parameters);

  virtual ~NearestNodeDistanceAux() {}

  virtual void setup();

protected:
  virtual Real computeValue();

  NearestNodeLocator & _nearest_node;
};

#endif //NEARESTNODEDISTANCEAUX_H
