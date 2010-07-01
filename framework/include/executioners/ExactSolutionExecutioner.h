#ifndef EXACTSOLEXEC_H
#define EXACTSOLEXEC_H

//libmesh includes
#include "exact_solution.h"

//moose includes
#include "Steady.h"
#include "Functor.h"

// Forward Declarations
class ExactSolutionExecutioner;

template<>
InputParameters validParams<ExactSolutionExecutioner>();

/**
 * This class overrides postSolve() to print the dofs and the l2 norm of the
 * Moose solution and the exact solution for every solve.
 */
class ExactSolutionExecutioner: public Steady
{
public:

  ExactSolutionExecutioner(std::string name, MooseSystem & moose_system, InputParameters parameters);

  /**
   * Function pointer to evaluate the exact solution at a point.
   */
  Number exactSolution(const Point& p,
                const Parameters& Parameters,     // not needed
                const std::string& sys_name,      // not needed
                const std::string& unknown_name); // not needed

protected:
  /**
   * Override to print the l2 norm between the simulation and the exact solution
   */
  virtual void postSolve();

private:
  ExactSolution _exact;
  Functor _functor;
};

#endif //EXACTSOLEXEC_H
