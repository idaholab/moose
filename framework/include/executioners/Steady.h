#ifndef STEADY_H
#define STEADY_H

#include "Moose.h"
#include "Executioner.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>


// Forward Declarations
class Steady;
template<>
InputParameters validParams<Steady>();

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
  Steady(std::string name, MooseSystem & moose_system, InputParameters parameters);

  /**
   * This will call solve() on the NonlinearSystem.
   */
  virtual void execute();

protected:

  /**
   * Whether or not the last solve converged.
   */
  virtual bool lastSolveConverged();

  int _max_r_steps;
};

#endif //STEADY_H
