/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

//Moose includes
#include "MooseSystem.h"
#include "ExactSolutionExecutioner.h"

namespace NecessaryFuncPointer
{
  Number exactSolution(const Point& p,
                const Parameters& Parameters,
                const std::string& sys_name,
                const std::string& unknown_name);

  RealGradient exactGrad(const Point& p,
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

  params.addParam<bool>("h1_error", false, "Set to true if you want to compute the h1 Error. If you do this you must supply the gradient of the analytic solution.");

  params.addParam<int>("extra_quadrature_order", 0, "Set this value if you want to integrate using this quadrature order (higher than necessary)");
  return params;
}

ExactSolutionExecutioner::ExactSolutionExecutioner(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Steady(name, moose_system, parameters),
  _exact(*moose_system.getEquationSystems()),
  _func(getFunction("function")),
  _unknowns(getParam<std::vector<std::string> >("unknowns")),
  _output_norms(false),
  _compute_h1(getParam<bool>("h1_error"))
{
  //set a pointer to this object in the parameters, so the callback has
  //an object to call exactSolution() on
  //PJJ TODO: index by the name of the system and variable so it can be different for u and v, etc
  moose_system.getEquationSystems()->parameters.set<ExactSolutionExecutioner*>("ptr") = this;

  //attach a function pointer, this will grab the object set in the parameter
  //above to call the exactSolution() member function
  _exact.attach_exact_value(NecessaryFuncPointer::exactSolution);
  if (_compute_h1)
    _exact.attach_exact_deriv(NecessaryFuncPointer::exactGrad);

  int quad_order = getParam<int>("extra_quadrature_order");
  if (quad_order != 0)
    _exact.extra_quadrature_order(quad_order);

  //open the file to write output to if the user wants it
  std::string file = getParam<std::string>("norm_file");
  if( file != "" )
  {
    _output_norms = true;
    _out_file.open(file.c_str(), std::ios::out | std::ios::trunc );
    _out_file << "var\tdofs\tl2 norm";
    if (_compute_h1)
      _out_file << "    \th1 norm";
    _out_file << "\n";
  }
}

ExactSolutionExecutioner::~ExactSolutionExecutioner()
{
  if (_output_norms)
    _out_file.close();
}

Number
ExactSolutionExecutioner::exactSolution(const Point& p, const Parameters& /*parameters*/, const std::string& /*sys_name*/, const std::string& /*unknown_name*/)
{
  //TODO is this the right way to check dimensions?
  Real x = p(0);
  Real y = (LIBMESH_DIM > 1) ? p(1) : 0;
  Real z = (LIBMESH_DIM > 2) ? p(2) : 0;
  Real t = _moose_system._t;
  return _func.value(t, x, y, z);
}

RealGradient
ExactSolutionExecutioner::exactGrad(const Point& p, const Parameters& /*parameters*/, const std::string& /*sys_name*/, const std::string& /*unknown_name*/)
{
  //TODO is this the right way to check dimensions?
  Real x = p(0);
  Real y = (LIBMESH_DIM > 1) ? p(1) : 0;
  Real z = (LIBMESH_DIM > 2) ? p(2) : 0;
  Real t = _moose_system._t;
  return _func.gradient(t, x, y, z);
}

void
ExactSolutionExecutioner::postSolve()
{
  for (unsigned int i = 0; i < _unknowns.size(); i++)
  {
    std::string var = _unknowns[i];
    _exact.compute_error("NonlinearSystem", var);
    Real l2 = _exact.l2_error("NonlinearSystem", var);
    Real h1;
    int dofs = _moose_system.getEquationSystems()->n_dofs();

    std::cout << var << " dofs: " << dofs << "\n";
    std::cout << var << " l2:   " << l2 << "\n";
    if (_compute_h1)
    {
      h1 = _exact.h1_error("NonlinearSystem", var);
      std::cout << var << " h1:   " << h1 << "\n";
    }

    if( _output_norms )
    {
      _out_file << var << "\t" << dofs << "\t" << l2;
      if (_compute_h1)
        _out_file << "     \t" << h1;
      _out_file << std::endl;
    }
  }
}

//function pointer
Number
NecessaryFuncPointer::exactSolution(const Point& p, const Parameters& parameters, const std::string& sys_name, const std::string& unknown_name)
{
  return parameters.get<ExactSolutionExecutioner*>("ptr")->exactSolution(p, parameters, sys_name, unknown_name);
}

RealGradient
NecessaryFuncPointer::exactGrad(const Point& p, const Parameters& parameters, const std::string& sys_name, const std::string& unknown_name)
{
  return parameters.get<ExactSolutionExecutioner*>("ptr")->exactGrad(p, parameters, sys_name, unknown_name);
}
