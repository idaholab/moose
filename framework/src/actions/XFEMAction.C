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

#include "XFEMAction.h"

#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "XFEM.h"
#include "Executioner.h"
#include "MooseEnum.h"
#include "Parser.h"
#include "Factory.h"

#include "XFEM_geometric_cut_2d.h"
#include "XFEM_square_cut.h"
#include "XFEM_circle_cut.h"
#include "XFEM_ellipse_cut.h"

// libMesh includes
#include "libmesh/transient_system.h"
#include "libmesh/string_to_enum.h"

template<>
InputParameters validParams<XFEMAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<std::string>("cut_type", "line_segment_2d", "The type of XFEM cuts");
  params.addParam<std::vector<Real> >("cut_data","Data for XFEM geometric cuts");
  params.addParam<std::vector<Real> >("cut_scale","X,Y scale factors for XFEM geometric cuts");
  params.addParam<std::vector<Real> >("cut_translate","X,Y translations for XFEM geometric cuts");
  params.addParam<std::string>("qrule", "volfrac", "XFEM quadrature rule to use");
  params.addParam<bool>("output_cut_plane",false,"Output the XFEM cut plane and volume fraction");
  params.addParam<bool>("use_crack_growth_increment", false, "Use fixed crack growth increment");
  params.addParam<Real>("crack_growth_increment", 0.1, "Crack growth increment");
  return params;
}

XFEMAction::XFEMAction(InputParameters params) :
    Action(params),
    _xfem_cut_type(getParam<std::string>("cut_type")),
    _xfem_qrule(getParam<std::string>("qrule")),
    _xfem_cut_plane(false),
    _xfem_use_crack_growth_increment(getParam<bool>("use_crack_growth_increment")),
    _xfem_crack_growth_increment(getParam<Real>("crack_growth_increment"))
{
   _order = "CONSTANT";
   _family = "MONOMIAL";
   if (isParamValid("output_cut_plane"))
     _xfem_cut_plane = getParam<bool>("output_cut_plane");
}

void
XFEMAction::act()
{

  XFEM* xfem = _problem->get_xfem();
  _problem->setUseXFEM();

  if(_current_task == "setup_xfem"){

    _xfem_cut_data = getParam<std::vector<Real> >("cut_data");

    xfem->set_xfem_qrule(_xfem_qrule);

    xfem->set_crack_growth_method(_xfem_use_crack_growth_increment, _xfem_crack_growth_increment);

    if (_xfem_cut_type == "line_segment_2d")
    {
      if (_xfem_cut_data.size() % 6 != 0)
      mooseError("Length of XFEM_cuts must be a multiple of 6.");

      unsigned int num_cuts = _xfem_cut_data.size()/6;

      std::vector<Real> trans;
      if (isParamValid("cut_translate"))
      {
        trans = getParam<std::vector<Real> >("cut_translate");
      }
      else
      {
        trans.push_back(0.0);
        trans.push_back(0.0);
      }

      std::vector<Real> scale;
      if (isParamValid("cut_scale"))
      {
        scale = getParam<std::vector<Real> >("cut_scale");
      }
      else
      {
        scale.push_back(1.0);
        scale.push_back(1.0);
      }

      for (unsigned int i = 0; i < num_cuts; ++i)
      {
        Real x0 = (_xfem_cut_data[i*6+0]+trans[0])*scale[0];
        Real y0 = (_xfem_cut_data[i*6+1]+trans[1])*scale[1];
        Real x1 = (_xfem_cut_data[i*6+2]+trans[0])*scale[0];
        Real y1 = (_xfem_cut_data[i*6+3]+trans[1])*scale[1];
        Real t0 = _xfem_cut_data[i*6+4];
        Real t1 = _xfem_cut_data[i*6+5];
        xfem->addGeometricCut(new XFEM_geometric_cut_2d( x0, y0, x1, y1, t0, t1));
      }
    }
    else if (_xfem_cut_type == "square_cut_3d")
    {
      if(_xfem_cut_data.size() % 12 != 0)
        mooseError("Length of XFEM_cuts must be 12 when square_cut_3d");

      unsigned int num_cuts = _xfem_cut_data.size()/12;
      std::vector<Real> square_cut_data(12);
      for(unsigned i = 0; i < num_cuts; ++i){
        for(unsigned j = 0; j < 12; j++){
          square_cut_data[j] = _xfem_cut_data[i*12+j];
        }
        xfem->addGeometricCut(new XFEM_square_cut(square_cut_data));
      }
    }
    else if (_xfem_cut_type == "circle_cut_3d")
    {
       if(_xfem_cut_data.size() % 9 != 0)
         mooseError("Length of XFEM_cuts must be 9 when circle_cut_3d");

       unsigned int num_cuts = _xfem_cut_data.size()/9;
       std::vector<Real> circle_cut_data(9);
       for(unsigned i = 0; i < num_cuts; ++i){
         for(unsigned j = 0; j < 9; j++){
           circle_cut_data[j] = _xfem_cut_data[i*9+j];
         }
         xfem->addGeometricCut(new XFEM_circle_cut(circle_cut_data));
       }
    }
    else if (_xfem_cut_type == "ellipse_cut_3d")
    {
      if(_xfem_cut_data.size() % 9 != 0)
        mooseError("Length of XFEM_cuts must be 9 when ellipse_cut_3d");

      unsigned int num_cuts = _xfem_cut_data.size()/9;
      std::vector<Real> ellipse_cut_data(9);
      for(unsigned i = 0; i < num_cuts; ++i){
        for(unsigned j = 0; j < 9; j++){
          ellipse_cut_data[j] = _xfem_cut_data[i*9+j];
        }
        xfem->addGeometricCut(new XFEM_ellipse_cut(ellipse_cut_data));
      }
    }
    else
      mooseError("unrecognized XFEM cut type");
  }
  else if (_current_task == "add_aux_variable" && _xfem_cut_plane)
  {
    _problem->addAuxVariable("xfem_cut_origin_x",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_cut_origin_y",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_cut_origin_z",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_cut_normal_x",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_cut_normal_y",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_cut_normal_z",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));

    _problem->addAuxVariable("xfem_cut2_origin_x",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_cut2_origin_y",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_cut2_origin_z",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_cut2_normal_x",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_cut2_normal_y",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
    _problem->addAuxVariable("xfem_cut2_normal_z",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));

    _problem->addAuxVariable("xfem_volfrac",FEType(Utility::string_to_enum<Order>(_order),Utility::string_to_enum<FEFamily>(_family)));
  }
  else if (_current_task == "add_aux_kernel" && _xfem_cut_plane)
  {
    InputParameters params = _factory.getValidParams("XFEMVolFracAux");
    params.set<MultiMooseEnum>("execute_on") = "timestep_begin";
    params.set<AuxVariableName>("variable") = "xfem_volfrac";
    _problem->addAuxKernel("XFEMVolFracAux","xfem_volfrac",params);

    params = _factory.getValidParams("XFEMCutPlaneAux");
    params.set<MultiMooseEnum>("execute_on") = "timestep_end";

    // first cut plane
    params.set<unsigned int>("plane_id") = 0;

    params.set<AuxVariableName>("variable") = "xfem_cut_origin_x";
    params.set<MooseEnum>("quantity") = "origin_x";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut_origin_x", params);

    params.set<AuxVariableName>("variable") = "xfem_cut_origin_y";
    params.set<MooseEnum>("quantity") = "origin_y";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut_origin_y", params);

    params.set<AuxVariableName>("variable") = "xfem_cut_origin_z";
    params.set<MooseEnum>("quantity") = "origin_z";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut_origin_z", params);

    params.set<AuxVariableName>("variable") = "xfem_cut_normal_x";
    params.set<MooseEnum>("quantity") = "normal_x";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut_normal_x", params);

    params.set<AuxVariableName>("variable") = "xfem_cut_normal_y";
    params.set<MooseEnum>("quantity") = "normal_y";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut_normal_y", params);

    params.set<AuxVariableName>("variable") = "xfem_cut_normal_z";
    params.set<MooseEnum>("quantity") = "normal_z";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut_normal_z", params);

    // second cut plane
    params.set<unsigned int>("plane_id") = 1;

    params.set<AuxVariableName>("variable") = "xfem_cut2_origin_x";
    params.set<MooseEnum>("quantity") = "origin_x";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut2_origin_x", params);

    params.set<AuxVariableName>("variable") = "xfem_cut2_origin_y";
    params.set<MooseEnum>("quantity") = "origin_y";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut2_origin_y", params);

    params.set<AuxVariableName>("variable") = "xfem_cut2_origin_z";
    params.set<MooseEnum>("quantity") = "origin_z";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut2_origin_z", params);

    params.set<AuxVariableName>("variable") = "xfem_cut2_normal_x";
    params.set<MooseEnum>("quantity") = "normal_x";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut2_normal_x", params);

    params.set<AuxVariableName>("variable") = "xfem_cut2_normal_y";
    params.set<MooseEnum>("quantity") = "normal_y";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut2_normal_y", params);

    params.set<AuxVariableName>("variable") = "xfem_cut2_normal_z";
    params.set<MooseEnum>("quantity") = "normal_z";
    _problem->addAuxKernel("XFEMCutPlaneAux", "xfem_cut2_normal_z", params);
  }
}
