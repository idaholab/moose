#ifndef NEARESTNODEVALUEAUX_H
#define NEARESTNODEVALUEAUX_H

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

  NearestNodeLocator & _nearest_node;

  const NumericVector<Number> & _serialized_solution;

  unsigned int _paired_variable;
};

#endif //NEARESTNODEVALUEAUX_H
