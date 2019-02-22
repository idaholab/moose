#ifndef JACOBIANTESTACTION_H
#define JACOBIANTESTACTION_H

#include "TestAction.h"

class JacobianTestAction;

template <>
InputParameters validParams<JacobianTestAction>();

/**
 * Base class for adding common actions for Jacobian tests
 */
class JacobianTestAction : public TestAction
{
public:
  JacobianTestAction(InputParameters params);

protected:
  virtual void addPreconditioner() override;

  /// Finite differencing parameter
  const std::string _snes_test_err;
};

#endif /* JACOBIANTESTACTION_H */
