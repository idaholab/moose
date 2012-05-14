#ifndef EXCEPTIONSTEADY_H
#define EXCEPTIONSTEADY_H

#include "Steady.h"

class ExceptionSteady;

template<>
InputParameters validParams<ExceptionSteady>();

/**
 * Test executioner to show exception handling
 */
class ExceptionSteady : public Steady
{
public:
  ExceptionSteady(const std::string & name, InputParameters parameters);
  virtual ~ExceptionSteady();

  /**
   * This will call solve() on the NonlinearSystem.
   */
  virtual void execute();
};

#endif /* EXCEPTIONSTEADY_H */
