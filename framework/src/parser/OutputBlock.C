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

#include "OutputBlock.h"
#include "Moose.h"

#include "exodusII_io.h"
#include "mesh.h"

template<>
InputParameters validParams<OutputBlock>()
{
  InputParameters params = validParams<ParserBlock>();

  params.addParam<std::string>("file_base", "out", "The desired solution output name without an extension");
  params.addParam<int>("interval", 1, "The iterval at which timesteps are output to the solution file");
  params.addParam<bool>("exodus", false, "Specifies that you would like Exodus output solution file(s)");
  params.addParam<bool>("gmv", false, "Specifies that you would like GMV output solution file(s)");
  params.addParam<bool>("tecplot", false, "Specifies that you would like Tecplot output solution files(s)");
  params.addParam<bool>("xda", false, "Specifies that you would like xda output solution files(s)");
  params.addParam<bool>("postprocessor_screen", true, "Specifies that you would like PostProcessor output to the screen (stdout)"); 
  params.addParam<bool>("postprocessor_csv", false, "Specifies that you would like a PostProcessor comma seperated values file"); 
  params.addParam<bool>("print_out_info", false, "Specifies that you would like to see more verbose output information on STDOUT");
  params.addParam<bool>("output_initial", false, "Requests that the initial condition is output to the solution file");
  return params;
}

OutputBlock::OutputBlock(const std::string & name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{
  // Register Output prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
  addPrereq("Preconditioning");
  addPrereq("AuxVariables");
  addPrereq("Kernels");
  addPrereq("AuxKernels");
  addPrereq("BCs");
  addPrereq("AuxBCs");
  addPrereq("Materials");
  addPrereq("Executioner");
}

void
OutputBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the OutputBlock Object\n";
#endif

  _moose_system._file_base = getParamValue<std::string>("file_base");
  _moose_system._interval = getParamValue<int>("interval");
  _moose_system._exodus_output = getParamValue<bool>("exodus");
  _moose_system._gmv_output = getParamValue<bool>("gmv");
  _moose_system._tecplot_output = getParamValue<bool>("tecplot");
  _moose_system._xda_output = getParamValue<bool>("xda");
  _moose_system._postprocessor_screen_output = getParamValue<bool>("postprocessor_screen");
  _moose_system._postprocessor_csv_output = getParamValue<bool>("postprocessor_csv");
  _moose_system._print_out_info = getParamValue<bool>("print_out_info");
  _moose_system._output_initial = getParamValue<bool>("output_initial");

  visitChildren();
}

