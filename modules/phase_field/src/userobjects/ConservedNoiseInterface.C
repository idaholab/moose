#include "ConservedNoiseInterface.h"

ConservedNoiseInterface::ConservedNoiseInterface(const std::string & name, InputParameters parameters) :
    ElementUserObject(name, parameters),
    _integral(0),
    _volume(0),
    _qp(0)
{
  /**
   * This call turns on Random Number generation for this object, it can be called either in
   * the constructor or in initialSetup().
   */
  setRandomResetFrequency(EXEC_TIMESTEP);
}
