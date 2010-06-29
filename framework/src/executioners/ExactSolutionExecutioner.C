//Moose includes
#include "MooseSystem.h"
#include "ExactSolutionExecutioner.h"

//terrible hack to allow us to pass a function pointer to ExactSolution
namespace FixMeTerrible
{
  ExactSolutionExecutioner * obj;

  Number exactSolution(const Point& p,
                const Parameters& Parameters,     // not needed
                const std::string& sys_name,      // not needed
                const std::string& unknown_name); // not needed
}

template<>
InputParameters validParams<ExactSolutionExecutioner>()
{
  InputParameters params = validParams<Steady>();
  params.addRequiredParam<std::string>("function", "The exact solution.");
  params.addParam<std::vector<std::string> >("vars", std::vector<std::string>(0), "The variables (excluding t,x,y,z) in the exact solution.");
  params.addParam<std::vector<Real> >("vals", std::vector<Real>(0), "The values that correspond to the variables.");
  return params;
}

ExactSolutionExecutioner::ExactSolutionExecutioner(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Steady(name, moose_system, parameters),
  _exact(*moose_system.getEquationSystems()),
  _functor(parameters.get<std::string>("function"),
           parameters.get<std::vector<std::string> >("vars"),
           parameters.get<std::vector<Real> >("vals"))
{
  FixMeTerrible::obj = this;
  _exact.attach_exact_value(FixMeTerrible::exactSolution);
}

Number
ExactSolutionExecutioner::exactSolution(const Point& p, const Parameters& Parameters, const std::string& sys_name, const std::string& unknown_name)
{
  //TODO is this the right way to check dimensions?
  Real x = p(0);
  Real y = (LIBMESH_DIM > 1) ? p(1) : 0;
  Real z = (LIBMESH_DIM > 2) ? p(2) : 0;
  Real t = _moose_system._t;
  return _functor(t, x, y, z);
}

void
ExactSolutionExecutioner::postSolve()
{
  std::cout << "dofs: " << _moose_system.getEquationSystems()->n_dofs() << "\n";

  //TODO will it always be called NonlinearSystem??
  _exact.compute_error("NonlinearSystem", "u");
  Real val = _exact.l2_error("NonlinearSystem", "u");

  std::cout << "norm: " << val << "\n";
}

//function pointer
Number
FixMeTerrible::exactSolution(const Point& p, const Parameters& Parameters, const std::string& sys_name, const std::string& unknown_name)
{
  return FixMeTerrible::obj->exactSolution(p, Parameters, sys_name, unknown_name);
}
