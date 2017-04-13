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

// MOOSE includes
#include "ParsedSubdomainMeshModifier.h"
#include "Conversion.h"
#include "MooseMesh.h"

// libmesh includes
#include "libmesh/fparser_ad.hh"

template <>
InputParameters
validParams<ParsedSubdomainMeshModifier>()
{
  InputParameters params = validParams<MeshModifier>();
  params += validParams<FunctionParserUtils>();
  params.addRequiredParam<std::string>("combinatorial_geometry",
                                       "Function expression encoding a combinatorial geometry");
  params.addRequiredParam<SubdomainID>("block_id",
                                       "Subdomain id to set for inside of the combinatorial");
  params.addParam<SubdomainName>("block_name",
                                 "Subdomain name to set for inside of the combinatorial");
  params.addParam<std::vector<SubdomainID>>("excluded_subdomain_ids",
                                            "A set of subdomain ids that will not changed even if "
                                            "they are inside/outside the combinatorial geometry");
  params.addParam<std::vector<std::string>>(
      "constant_names", "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  params.addClassDescription("MeshModifier that uses a parsed expression (combinatorial_geometry) "
                             "to determine if an element (aka its centroid) is inside the "
                             "combinatorial geometry and "
                             "assigns a new block id.");
  return params;
}

ParsedSubdomainMeshModifier::ParsedSubdomainMeshModifier(const InputParameters & parameters)
  : MeshModifier(parameters),
    FunctionParserUtils(parameters),
    _function(parameters.get<std::string>("combinatorial_geometry")),
    _block_id(parameters.get<SubdomainID>("block_id")),
    _excluded_ids(parameters.get<std::vector<SubdomainID>>("excluded_subdomain_ids"))
{
  // base function object
  _func_F = ADFunctionPtr(new ADFunction());

  // set FParser internal feature flags
  setParserFeatureFlags(_func_F);

  // add the constant expressions
  addFParserConstants(_func_F,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"));

  // parse function
  if (_func_F->Parse(_function, "x,y,z") >= 0)
    mooseError("Invalid function\n",
               _function,
               "\nin ParsedSubdomainMeshModifier ",
               name(),
               ".\n",
               _func_F->ErrorMsg());

  _func_params.resize(3);
}

void
ParsedSubdomainMeshModifier::modify()
{
  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError(
        "_mesh_ptr must be initialized before calling ParsedSubdomainMeshModifier::modify()");

  // Reference the the libMesh::MeshBase
  MeshBase & mesh = _mesh_ptr->getMesh();

  // Loop over the elements
  for (MeshBase::element_iterator el = mesh.active_elements_begin();
       el != mesh.active_elements_end();
       ++el)
  {
    _func_params[0] = (*el)->centroid()(0);
    _func_params[1] = (*el)->centroid()(1);
    _func_params[2] = (*el)->centroid()(2);
    bool contains = evaluate(_func_F);

    if (contains &&
        std::find(_excluded_ids.begin(), _excluded_ids.end(), (*el)->subdomain_id()) ==
            _excluded_ids.end())
      (*el)->subdomain_id() = _block_id;
  }

  // Assign block name, if provided
  if (isParamValid("block_name"))
    _mesh_ptr->getMesh().subdomain_name(_block_id) = getParam<SubdomainName>("block_name");
}
