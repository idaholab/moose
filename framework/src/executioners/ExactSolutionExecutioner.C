//Moose includes
#include "MooseSystem.h"
#include "ExactSolutionExecutioner.h"

namespace NecessaryFuncPointer
{
  Number exactSolution(const Point& p,
                const Parameters& Parameters,
                const std::string& sys_name,
                const std::string& unknown_name);
}

template<>
InputParameters validParams<ExactSolutionExecutioner>()
{
  InputParameters params = validParams<Steady>();
  params.addRequiredParam<std::string>("function", "The exact solution.");

  params.addParam<std::vector<std::string> >("unknowns", std::vector<std::string>(1, "u"), "List of the variables to compute dofs and norms for. Defaults to just u.");
  params.addParam<std::string>("norm_file", "", "If you pass a file this kernel will write out a table to this file containing: the variable, the dofs, and the l2 norm.");
  return params;
}

ExactSolutionExecutioner::ExactSolutionExecutioner(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Steady(name, moose_system, parameters),
  _exact(*moose_system.getEquationSystems()),
  _unknowns(parameters.get<std::vector<std::string> >("unknowns")),
  _func(getFunction("function")),
  _output_norms(false)
{
  //set a pointer to this object in the parameters, so the callback has
  //an object to call exactSolution() on
  //PJJ TODO: index by the name of the system and variable so it can be different for u and v, etc
  moose_system.getEquationSystems()->parameters.set<ExactSolutionExecutioner*>("ptr") = this;

  //attach a function pointer, this will grab the object set in the parameter
  //above to call the exactSolution() member function
  _exact.attach_exact_value(NecessaryFuncPointer::exactSolution);

  //open the file to write output to if the user wants it
  std::string file = parameters.get<std::string>("norm_file");
  if( file != "" )
  {
    _output_norms = true;
    _out_file.open(file.c_str(), std::ios::out | std::ios::trunc );
    _out_file << "var\tdofs\tl2 norm\n";
  }
}

ExactSolutionExecutioner::~ExactSolutionExecutioner()
{
  if (_output_norms)
    _out_file.close();
}

Number
ExactSolutionExecutioner::exactSolution(const Point& p, const Parameters& parameters, const std::string& sys_name, const std::string& unknown_name)
{
  //TODO is this the right way to check dimensions?
  Real x = p(0);
  Real y = (LIBMESH_DIM > 1) ? p(1) : 0;
  Real z = (LIBMESH_DIM > 2) ? p(2) : 0;
  Real t = _moose_system._t;
  return _func(t, x, y, z);
}

void
ExactSolutionExecutioner::postSolve()
{
  for (int i = 0; i < _unknowns.size(); i++)
  {
    std::string var = _unknowns[i];
    _exact.compute_error("NonlinearSystem", var);
    Real val = _exact.l2_error("NonlinearSystem", var);
    int dofs = _moose_system.getEquationSystems()->n_dofs();

    std::cout << var << " dofs: " << dofs << "\n";
    std::cout << var << " norm: " << val << "\n";

    if( _output_norms )
      _out_file << var << '\t' << dofs << '\t' << val << std::endl;
  }
}

//function pointer
Number
NecessaryFuncPointer::exactSolution(const Point& p, const Parameters& parameters, const std::string& sys_name, const std::string& unknown_name)
{
  return parameters.get<ExactSolutionExecutioner*>("ptr")->exactSolution(p, parameters, sys_name, unknown_name);
}
