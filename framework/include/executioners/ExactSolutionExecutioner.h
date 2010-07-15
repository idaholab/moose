#ifndef EXACTSOLEXEC_H
#define EXACTSOLEXEC_H

#include <ios>
#include <iostream>
#include <fstream>

//libmesh includes
#include "exact_solution.h"

//moose includes
#include "Steady.h"
#include "Function.h"

// Forward Declarations
class ExactSolutionExecutioner;

template<>
InputParameters validParams<ExactSolutionExecutioner>();

/**
 * This class overrides postSolve() to print the dofs and the l2 norm of the
 * Moose solution and the exact solution for every solve.  It will also output
 * those values to a file if you specify it in the input file.
 */
class ExactSolutionExecutioner : public Steady
{
public:
  ExactSolutionExecutioner(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~ExactSolutionExecutioner();

  /**
   * Function pointer to evaluate the exact solution at a point.
   */
  Number exactSolution(const Point& p,
                const Parameters& Parameters,
                const std::string& sys_name,
                const std::string& unknown_name);

  /**
   * Function pointer to evaluate the exact gradient at a point.
   */
  RealGradient exactGrad(const Point& p,
                const Parameters& Parameters,
                const std::string& sys_name,
                const std::string& unknown_name);

protected:
  /**
   * Override to print the l2 norm between the simulation and the exact solution
   */
  virtual void postSolve();

private:
  ExactSolution _exact;
  Function & _func;

  std::vector<std::string> _unknowns;

  std::ofstream _out_file;
  bool _output_norms;
  bool _compute_h1;
};

#endif //EXACTSOLEXEC_H
