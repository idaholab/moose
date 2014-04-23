#include "JIntegralAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "libmesh/string_to_enum.h"
#include "CrackFrontDefinition.h"

template<>
InputParameters validParams<JIntegralAction>()
{
  InputParameters params = validParams<Action>();
  addCrackFrontDefinitionParams(params);
  params.addParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where this boundary condition applies");
  params.addParam<std::string>("order", "FIRST",  "Specifies the order of the FE shape function to use for q AuxVariables");
  params.addParam<std::string>("family", "LAGRANGE", "Specifies the family of FE shape functions to use for q AuxVariables");
  params.addRequiredParam<std::vector<Real> >("radius_inner", "Inner radius for volume integral domain");
  params.addRequiredParam<std::vector<Real> >("radius_outer", "Outer radius for volume integral domain");
  return params;
}

JIntegralAction::JIntegralAction(const std::string & name, InputParameters params):
  Action(name, params),
  _boundary_names(getParam<std::vector<BoundaryName> >("boundary")),
  _order(getParam<std::string>("order")),
  _family(getParam<std::string>("family")),
  _direction_method_moose_enum(getParam<MooseEnum>("crack_direction_method")),
  _end_direction_method_moose_enum(getParam<MooseEnum>("crack_end_direction_method")),
  _have_crack_direction_vector(false),
  _have_crack_direction_vector_end_1(false),
  _have_crack_direction_vector_end_2(false),
  _treat_as_2d(getParam<bool>("2d")),
  _axis_2d(getParam<unsigned int>("axis_2d")),
  _radius_inner(getParam<std::vector<Real> >("radius_inner")),
  _radius_outer(getParam<std::vector<Real> >("radius_outer")),
  _use_displaced_mesh(false)
{
  if (isParamValid("crack_direction_vector"))
  {
    _crack_direction_vector = getParam<RealVectorValue>("crack_direction_vector");
    _have_crack_direction_vector = true;
  }
  if (isParamValid("crack_direction_vector_end_1"))
  {
    _crack_direction_vector_end_1 = getParam<RealVectorValue>("crack_direction_vector_end_1");
    _have_crack_direction_vector_end_1 = true;
  }
  if (isParamValid("crack_direction_vector_end_2"))
  {
    _crack_direction_vector_end_2 = getParam<RealVectorValue>("crack_direction_vector_end_2");
    _have_crack_direction_vector_end_2 = true;
  }
  if (isParamValid("crack_mouth_boundary"))
  {
    _crack_mouth_boundary_names = getParam<std::vector<BoundaryName> >("crack_mouth_boundary");
  }
  if (_radius_inner.size() != _radius_outer.size())
  {
    mooseError("Number of entries in 'radius_inner' and 'radius_outer' must match.");
  }
}

JIntegralAction::~JIntegralAction()
{
}

void
JIntegralAction::act()
{
  const std::string uo_name("crackFrontDefinition");
  const std::string ak_base_name("q");
  const std::string av_base_name("q");
  const std::string pp_base_name("J");
  const unsigned int num_crack_front_nodes = calcNumCrackFrontNodes();

  if (_current_action == "add_user_object")
  {
    const std::string uo_type_name("CrackFrontDefinition");

    InputParameters params = _factory.getValidParams(uo_type_name);
    params.set<MooseEnum>("execute_on") = "initial";
    params.set<MooseEnum>("crack_direction_method") = _direction_method_moose_enum;
    params.set<MooseEnum>("crack_end_direction_method") = _end_direction_method_moose_enum;
    if (_have_crack_direction_vector)
    {
      params.set<RealVectorValue>("crack_direction_vector") = _crack_direction_vector;
    }
    if (_have_crack_direction_vector_end_1)
    {
      params.set<RealVectorValue>("crack_direction_vector_end_1") = _crack_direction_vector_end_1;
    }
    if (_have_crack_direction_vector_end_2)
    {
      params.set<RealVectorValue>("crack_direction_vector_end_2") = _crack_direction_vector_end_2;
    }
    if (_crack_mouth_boundary_names.size() != 0)
    {
      params.set<std::vector<BoundaryName> >("crack_mouth_boundary") = _crack_mouth_boundary_names;
    }
    params.set<bool>("2d") = _treat_as_2d;
    params.set<unsigned int>("axis_2d") = _axis_2d;
    params.set<std::vector<BoundaryName> >("boundary") = _boundary_names;
    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

    _problem->addUserObject(uo_type_name, uo_name, params);
  }
  else if (_current_action == "add_aux_variable")
  {
    for (unsigned int ring_index=0; ring_index<_radius_inner.size(); ++ring_index)
    {
      if (_treat_as_2d)
      {
        std::ostringstream av_name_stream;
        av_name_stream<<av_base_name<<"_"<<ring_index+1;
        _problem->addAuxVariable(av_name_stream.str(),
                                 FEType(Utility::string_to_enum<Order>(_order),
                                        Utility::string_to_enum<FEFamily>(_family)));
      }
      else
      {
        for (unsigned int cfn_index=0; cfn_index<num_crack_front_nodes; ++cfn_index)
        {
          std::ostringstream av_name_stream;
          av_name_stream<<av_base_name<<"_"<<cfn_index+1<<"_"<<ring_index+1;
          _problem->addAuxVariable(av_name_stream.str(),
                                   FEType(Utility::string_to_enum<Order>(_order),
                                          Utility::string_to_enum<FEFamily>(_family)));
        }
      }
    }
  }
  else if (_current_action == "add_aux_kernel")
  {
    const std::string ak_type_name("qFunctionJIntegral");
    InputParameters params = _factory.getValidParams(ak_type_name);
    params.set<MooseEnum>("execute_on") = "initial";
    params.set<UserObjectName>("crack_front_definition") = uo_name;
    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;

    for (unsigned int ring_index=0; ring_index<_radius_inner.size(); ++ring_index)
    {
      params.set<Real>("j_integral_radius_inner") = _radius_inner[ring_index];
      params.set<Real>("j_integral_radius_outer") = _radius_outer[ring_index];
      if (_treat_as_2d)
      {
        std::ostringstream ak_name_stream;
        ak_name_stream<<ak_base_name<<"_"<<ring_index+1;
        std::ostringstream av_name_stream;
        av_name_stream<<av_base_name<<"_"<<ring_index+1;
        params.set<AuxVariableName>("variable") = av_name_stream.str();
        _problem->addAuxKernel(ak_type_name, ak_name_stream.str(), params);
      }
      else
      {
        for (unsigned int cfn_index=0; cfn_index<num_crack_front_nodes; ++cfn_index)
        {
          std::ostringstream ak_name_stream;
          ak_name_stream<<ak_base_name<<"_"<<cfn_index+1<<"_"<<ring_index+1;
          std::ostringstream av_name_stream;
          av_name_stream<<av_base_name<<"_"<<cfn_index+1<<"_"<<ring_index+1;
          params.set<AuxVariableName>("variable") = av_name_stream.str();
          params.set<unsigned int>("crack_front_node_index") = cfn_index;
          _problem->addAuxKernel(ak_type_name, ak_name_stream.str(), params);
        }
      }
    }
  }
  else if (_current_action == "add_postprocessor")
  {
    const std::string pp_type_name("JIntegral");
    InputParameters params = _factory.getValidParams(pp_type_name);
    params.set<MooseEnum>("execute_on") = "timestep";
    params.set<UserObjectName>("crack_front_definition") = uo_name;
    params.set<bool>("use_displaced_mesh") = _use_displaced_mesh;
    for (unsigned int ring_index=0; ring_index<_radius_inner.size(); ++ring_index)
    {
      if (_treat_as_2d)
      {
        std::ostringstream av_name_stream;
        av_name_stream<<av_base_name<<"_"<<ring_index+1;
        std::ostringstream pp_name_stream;
        pp_name_stream<<pp_base_name<<"_"<<ring_index+1;
        std::vector<VariableName> qvars;
        qvars.push_back(av_name_stream.str());
        params.set<std::vector<VariableName> >("q") = qvars;
        _problem->addPostprocessor(pp_type_name,pp_name_stream.str(),params);
      }
      else
      {
        for (unsigned int cfn_index=0; cfn_index<num_crack_front_nodes; ++cfn_index)
        {
          std::ostringstream av_name_stream;
          av_name_stream<<av_base_name<<"_"<<cfn_index+1<<"_"<<ring_index+1;
          std::ostringstream pp_name_stream;
          pp_name_stream<<pp_base_name<<"_"<<cfn_index+1<<"_"<<ring_index+1;
          std::vector<VariableName> qvars;
          qvars.push_back(av_name_stream.str());
          params.set<std::vector<VariableName> >("q") = qvars;
          params.set<unsigned int>("crack_front_node_index") = cfn_index;
          _problem->addPostprocessor(pp_type_name,pp_name_stream.str(),params);
        }
      }
    }
  }
}

unsigned int
JIntegralAction::calcNumCrackFrontNodes()
{
  std::vector<BoundaryID> bids = _mesh->getBoundaryIDs(_boundary_names,true);
  std::set<unsigned int> nodes;

  ConstBndNodeRange & bnd_nodes = *_mesh->getBoundaryNodeRange();
  for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
  {
    const BndNode * bnode = *nd;
    BoundaryID boundary_id = bnode->_bnd_id;

    for (unsigned int ibid=0; ibid<bids.size(); ++ibid)
    {
      if (boundary_id == bids[ibid])
      {
        nodes.insert(bnode->_node->id());
        break;
      }
    }
  }
  return nodes.size();
}
