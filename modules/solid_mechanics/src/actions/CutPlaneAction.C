#include "CutPlaneAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<CutPlaneAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("order", "CONSTANT",  "Specifies the order of the FE shape function to use for q AuxVariables");
  params.addParam<std::string>("family", "MONOMIAL", "Specifies the family of FE shape functions to use for q AuxVariables");
  return params;
}

CutPlaneAction::CutPlaneAction(const std::string & name, InputParameters params):
  Action(name, params),
  _order(getParam<std::string>("order")),
  _family(getParam<std::string>("family"))
{
}

CutPlaneAction::~CutPlaneAction()
{
}

void
CutPlaneAction::act()
{
  if (_current_task == "add_aux_variable")
  {
    _problem->addAuxVariable("xfem_first_cut_origin_x",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_first_cut_origin_y",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_first_cut_origin_z",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_first_cut_normal_x",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_first_cut_normal_y",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_first_cut_normal_z",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));

    _problem->addAuxVariable("xfem_second_cut_origin_x",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_second_cut_origin_y",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_second_cut_origin_z",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_second_cut_normal_x",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_second_cut_normal_y",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_second_cut_normal_z",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
  }
  else if (_current_task == "add_aux_kernel")
  {
    InputParameters params = _factory.getValidParams("XFEMCutPlaneAux");
    params.set<MultiMooseEnum>("execute_on") = "timestep_end";

    // first cut plane
    params.set<unsigned int>("plane_id") = 0;

    params.set<AuxVariableName>("variable") = "xfem_first_cut_origin_x";
    params.set<MooseEnum>("quantity") = "origin_x";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_first_cut_origin_x", params);

    params.set<AuxVariableName>("variable") = "xfem_first_cut_origin_y";
    params.set<MooseEnum>("quantity") = "origin_y";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_first_cut_origin_y", params);

    params.set<AuxVariableName>("variable") = "xfem_first_cut_origin_z";
    params.set<MooseEnum>("quantity") = "origin_z";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_first_cut_origin_z", params);

    params.set<AuxVariableName>("variable") = "xfem_first_cut_normal_x";
    params.set<MooseEnum>("quantity") = "normal_x";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_first_cut_normal_x", params);

    params.set<AuxVariableName>("variable") = "xfem_first_cut_normal_y";
    params.set<MooseEnum>("quantity") = "normal_y";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_first_cut_normal_y", params);

    params.set<AuxVariableName>("variable") = "xfem_first_cut_normal_z";
    params.set<MooseEnum>("quantity") = "normal_z";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_first_cut_normal_z", params);

    // second cut plane
    params.set<unsigned int>("plane_id") = 1;

    params.set<AuxVariableName>("variable") = "xfem_second_cut_origin_x";
    params.set<MooseEnum>("quantity") = "origin_x";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_second_cut_origin_x", params);

    params.set<AuxVariableName>("variable") = "xfem_second_cut_origin_y";
    params.set<MooseEnum>("quantity") = "origin_y";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_second_cut_origin_y", params);

    params.set<AuxVariableName>("variable") = "xfem_second_cut_origin_z";
    params.set<MooseEnum>("quantity") = "origin_z";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_second_cut_origin_z", params);

    params.set<AuxVariableName>("variable") = "xfem_second_cut_normal_x";
    params.set<MooseEnum>("quantity") = "normal_x";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_second_cut_normal_x", params);

    params.set<AuxVariableName>("variable") = "xfem_second_cut_normal_y";
    params.set<MooseEnum>("quantity") = "normal_y";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_second_cut_normal_y", params);

    params.set<AuxVariableName>("variable") = "xfem_second_cut_normal_z";
    params.set<MooseEnum>("quantity") = "normal_z";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_second_cut_normal_z", params);
  }
}
