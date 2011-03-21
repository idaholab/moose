#ifndef STEADY_H_
#define STEADY_H_

#include "Moose.h"
#include "Executioner.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>


/**
 * Steady executioners usually only call "solve()" on the NonlinearSystem once.
 */
class Steady: public Executioner
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the Executioner in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  Steady(const std::string & name, InputParameters parameters);

  /**
   * This will call solve() on the NonlinearSystem.
   */
  virtual void execute();

protected:
  // not a real time, but we need to distinguish between initial condition and the solution (which we do by faking time)
  // this is only for outputting purposes
  Real & _time;
};

template<>
InputParameters validParams<Steady>();

#endif //STEADY_H
