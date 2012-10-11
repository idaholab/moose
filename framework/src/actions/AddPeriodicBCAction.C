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

#include "AddPeriodicBCAction.h"
#include "InputParameters.h"
#include "FunctionPeriodicBoundary.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "GeneratedMesh.h"

// LibMesh includes
#include "periodic_boundary.h" // translation PBCs provided by libmesh

template<>
InputParameters validParams<AddPeriodicBCAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::vector<std::string> >("auto_direction", "If using a generated mesh, you can specifiy just the dimension(s) you want to mark as periodic");

  params.addParam<BoundaryName>("primary", "Boundary ID associated with the primary boundary.");
  params.addParam<BoundaryName>("secondary", "Boundary ID associated with the secondary boundary.");
  params.addParam<RealVectorValue>("translation", "Vector that translates coordinates on the primary boundary to coordinates on the secondary boundary.");
  params.addParam<std::vector<std::string> >("transform_func", "Functions that specify the transformation");
  params.addParam<std::vector<std::string> >("inv_transform_func", "Functions that specify the inverse transformation");

  params.addParam<std::vector<std::string> >("variable", "Variable for the periodic boundary");
  return params;
}

AddPeriodicBCAction::AddPeriodicBCAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AddPeriodicBCAction::setPeriodicVars(PeriodicBoundaryBase & p, const std::vector<std::string> & var_names)
{
  NonlinearSystem & nl = _problem->getNonlinearSystem();

  for (std::vector<std::string>::const_iterator it = var_names.begin(); it != var_names.end(); ++it)
    p.set_variable(nl.getVariable(0, (*it)).number());
}

bool
AddPeriodicBCAction::autoTranslationBoundaries()
{
  if (isParamValid("auto_direction"))
  {
    GeneratedMesh *gen_mesh = dynamic_cast<GeneratedMesh *>(&_problem->mesh());
    if (!gen_mesh)
      mooseError("\"auto_direction\" is only supported for type \"GeneratedMesh\"");

    NonlinearSystem & nl = _problem->getNonlinearSystem();
    std::vector<std::string> auto_dirs = getParam<std::vector<std::string> >("auto_direction");

    unsigned int dim_offset = gen_mesh->dimension() - 2;
    for (unsigned int i=0; i<auto_dirs.size(); ++i)
    {
      if (auto_dirs[i] == "X" || auto_dirs[i] == "x")
      {
        PeriodicBoundary p(RealVectorValue(gen_mesh->dimensionWidth(0), 0, 0));
        p.myboundary = _paired_boundary_map[0][dim_offset][0];
        p.pairedboundary = _paired_boundary_map[0][dim_offset][1];
        setPeriodicVars(p, getParam<std::vector<std::string> >("variable"));
        nl.dofMap().add_periodic_boundary(p);
      }
      else if (auto_dirs[i] == "Y" || auto_dirs[i] == "y")
      {
        PeriodicBoundary p(RealVectorValue(0, gen_mesh->dimensionWidth(1), 0));
        p.myboundary = _paired_boundary_map[1][dim_offset][0];
        p.pairedboundary = _paired_boundary_map[1][dim_offset][1];
        setPeriodicVars(p, getParam<std::vector<std::string> >("variable"));
        nl.dofMap().add_periodic_boundary(p);
      }
      else if (auto_dirs[i] == "Z" || auto_dirs[i] == "z")
      {
        if (dim_offset == 0)
          mooseError("Cannot wrap 'Z' direction when using a 2D mesh");
        PeriodicBoundary p(RealVectorValue(0., 0., gen_mesh->dimensionWidth(2)));
        p.myboundary = _paired_boundary_map[2][dim_offset][0];
        p.pairedboundary = _paired_boundary_map[2][dim_offset][1];
        setPeriodicVars(p, getParam<std::vector<std::string> >("variable"));
        nl.dofMap().add_periodic_boundary(p);
      }
    }
    return true;
  }
  return false;
}

void
AddPeriodicBCAction::act()
{
  NonlinearSystem & nl = _problem->getNonlinearSystem();

  if (autoTranslationBoundaries())
    return;

  if (_pars.isParamValid("translation"))
  {
    RealVectorValue translation = getParam<RealVectorValue>("translation");

    PeriodicBoundary p(translation);
    p.myboundary = _problem->mesh().getBoundaryID(getParam<BoundaryName>("primary"));
    p.pairedboundary = _problem->mesh().getBoundaryID(getParam<BoundaryName>("secondary"));
    setPeriodicVars(p, getParam<std::vector<std::string> >("variable"));

    nl.dofMap().add_periodic_boundary(p);
  }
  else if (getParam<std::vector<std::string> >("transform_func") != std::vector<std::string>())
  {
    std::vector<std::string> inv_fn_names = getParam<std::vector<std::string> >("inv_transform_func");
    std::vector<std::string> fn_names = getParam<std::vector<std::string> >("transform_func");

    // If the user provided a forward transformation, he must also provide an inverse -- we can't
    // form the inverse of an arbitrary function automatically...
    if (inv_fn_names == std::vector<std::string>())
      mooseError("You must provide an inv_transform_func for FunctionPeriodicBoundary!");

    FunctionPeriodicBoundary pb(*_problem, fn_names);
    pb.myboundary = _problem->mesh().getBoundaryID(getParam<BoundaryName>("primary"));
    pb.pairedboundary = _problem->mesh().getBoundaryID(getParam<BoundaryName>("secondary"));
    setPeriodicVars(pb, getParam<std::vector<std::string> >("variable"));

    FunctionPeriodicBoundary ipb(*_problem, inv_fn_names);
    ipb.myboundary = _problem->mesh().getBoundaryID(getParam<BoundaryName>("secondary"));   // these are swapped
    ipb.pairedboundary = _problem->mesh().getBoundaryID(getParam<BoundaryName>("primary")); // these are swapped
    setPeriodicVars(ipb, getParam<std::vector<std::string> >("variable"));

    // Add the pair of periodic boundaries to the dof map
    nl.dofMap().add_periodic_boundary(pb, ipb);
  }
  else
  {
    mooseError("You have to specify either 'auto_direction', 'translation' or 'trans_func' in your period boundary section '" + _name + "'");
  }
}

const Real AddPeriodicBCAction::_paired_boundary_map[3][2][2] =
{
  // X mappings (2D and 3D)
  {{3, 1}, {4, 2}},
  // Y mappings (2D and 3D)
  {{0, 2}, {1, 3}},
  // Z mappings (3D)
  {{-1, -1}, {0, 5}}
};
