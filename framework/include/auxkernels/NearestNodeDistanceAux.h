#ifndef NEARESTNODEDISTANCEAUX_H_
#define NEARESTNODEDISTANCEAUX_H_

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

  Moose::NearestNodeLocator & _nearest_node;
};

#endif //NEARESTNODEDISTANCEAUX_H_
