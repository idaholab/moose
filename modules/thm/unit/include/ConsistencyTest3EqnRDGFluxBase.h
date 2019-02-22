#ifndef CONSISTENCYTEST3EQNRDGFLUXBASE_H
#define CONSISTENCYTEST3EQNRDGFLUXBASE_H

#include "Test3EqnRDGObjectBase.h"

/**
 * Base class for testing consistency of a numerical flux for the 3-equation model.
 */
class ConsistencyTest3EqnRDGFluxBase : public Test3EqnRDGObjectBase
{
public:
  ConsistencyTest3EqnRDGFluxBase() : Test3EqnRDGObjectBase() {}

protected:
  virtual void test() override;
};

#endif /* CONSISTENCYTEST3EQNRDGFLUXBASE_H */
