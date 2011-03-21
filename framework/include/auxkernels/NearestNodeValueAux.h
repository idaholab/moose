#ifndef NEARESTNODEVALUEAUX_H_
#define NEARESTNODEVALUEAUX_H_

#include "AuxKernel.h"
#include "NearestNodeLocator.h"


//Forward Declarations
class NearestNodeValueAux;

template<>
InputParameters validParams<NearestNodeValueAux>();

/**
 * Constant auxiliary value
 */
class NearestNodeValueAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NearestNodeValueAux(const std::string & name, InputParameters parameters);

  virtual ~NearestNodeValueAux() {}

  virtual void setup();

protected:
  virtual Real computeValue();

  Moose::NearestNodeLocator & _nearest_node;

  unsigned int _paired_variable;
};

#endif //NEARESTNODEVALUEAUX_H_
