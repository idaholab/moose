/*************************************************/
/*           DO NOT MODIFY THIS HEADER           */
/*                                               */
/*                     BISON                     */
/*                                               */
/*    (c) 2015 Battelle Energy Alliance, LLC     */
/*            ALL RIGHTS RESERVED                */
/*                                               */
/*   Prepared by Battelle Energy Alliance, LLC   */
/*     Under Contract No. DE-AC07-05ID14517      */
/*     With the U. S. Department of Energy       */
/*                                               */
/*     See COPYRIGHT for full restrictions       */
/*************************************************/

#include "ChemicalCompositionAction.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MooseObjectAction.h"
#include "NonlinearSystemBase.h"
#include "Thermochimica.h"
#include "libmesh/string_to_enum.h"

registerMooseAction("BisonApp", ChemicalCompositionAction, "add_variable");
registerMooseAction("BisonApp", ChemicalCompositionAction, "add_aux_variable");
registerMooseAction("BisonApp", ChemicalCompositionAction, "add_ic");
registerMooseAction("BisonApp", ChemicalCompositionAction, "add_user_object");
registerMooseAction("BisonApp", ChemicalCompositionAction, "add_aux_kernel");

InputParameters
ChemicalCompositionAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Sets up the thermodynamic model and variables for the "
                             "thermochemistry using Thermochimica.");
  params.addParam<std::vector<std::string>>("elements", "List of chemical elements");
  params.addParam<FileName>("initial_values", "none", "The CSV file name with initial conditions.");
  params.addParam<std::string>(
      "var_type", "aux", "Variable type to be generated, aux (default) or base.");
  params.addParam<FileName>("thermofile", "none", "Thermodynamics model file");
  params.addParam<std::string>("tunit", "K", "Temperature Unit");
  params.addParam<std::string>("punit", "atm", "Pressure Unit");
  params.addParam<std::string>("munit", "moles", "Mass Unit");
  params.addParam<std::vector<std::string>>("output_phases", "List of phases to be output");
  params.addParam<std::vector<std::string>>(
      "output_species", "List species for which concentration in the phases is needed");
  params.addParam<std::vector<std::string>>(
      "element_potentials",
      "List of chemical elements for which chemical potentials are requested");
  return params;
}

ChemicalCompositionAction::ChemicalCompositionAction(InputParameters params)
  : Action(params),
    _elements(getParam<std::vector<std::string>>("elements")),
    _var_type(getParam<std::string>("var_type")),
    _inital_values_file_name(getParam<FileName>("initial_values")),
    _thermoFile(getParam<FileName>("thermofile")),
    _tunit(getParam<std::string>("tunit")),
    _punit(getParam<std::string>("punit")),
    _munit(getParam<std::string>("munit")),
    _phases(getParam<std::vector<std::string>>("output_phases")),
    _species(getParam<std::vector<std::string>>("output_species")),
    _element_potentials(getParam<std::vector<std::string>>("element_potentials"))

{
#ifndef THERMOCHIMICA_ENABLED
  mooseError("Thermochimica disabled");
#endif
}

void
ChemicalCompositionAction::act()
{
#ifdef THERMOCHIMICA_ENABLED
  if (_current_task == "add_variable" && _var_type == "base")
  {
    for (unsigned int i = 0; i < _elements.size(); i++)
    {
      std::string var_name = _elements[i];

      const bool second = _problem->mesh().hasSecondOrderElements();
      _problem->addVariable(var_name,
                            FEType(Utility::string_to_enum<Order>(second ? "SECOND" : "FIRST"),
                                   Utility::string_to_enum<FEFamily>("LAGRANGE")),
                            1.0);

      //      _problem->addVariable(var_name, FEType(FIRST, LAGRANGE),1.0);
    }

    for (unsigned int i = 0; i < _phases.size(); i++)
    {
      std::string var_name = _phases[i];

      const bool second = _problem->mesh().hasSecondOrderElements();
      _problem->addVariable(var_name,
                            FEType(Utility::string_to_enum<Order>(second ? "SECOND" : "FIRST"),
                                   Utility::string_to_enum<FEFamily>("LAGRANGE")),
                            1.0);
    }
    for (unsigned int i = 0; i < _species.size(); i++)
    {
      std::string var_name = _species[i];

      const bool second = _problem->mesh().hasSecondOrderElements();
      _problem->addVariable(var_name,
                            FEType(Utility::string_to_enum<Order>(second ? "SECOND" : "FIRST"),
                                   Utility::string_to_enum<FEFamily>("LAGRANGE")),
                            1.0);
    }
  }
  else if (_current_task == "add_aux_variable" && _var_type == "aux")
  {
    std::string aux_var_type = "MooseVariable";
    auto params = _factory.getValidParams(aux_var_type);
    params.set<MooseEnum>("order") = "FIRST";
    params.set<MooseEnum>("family") = "LAGRANGE";
    for (unsigned int i = 0; i < _elements.size(); i++)
    {
      std::string var_name = _elements[i];
      _problem->addAuxVariable(aux_var_type, var_name, params);
    }

    for (unsigned int i = 0; i < _phases.size(); i++)
    {
      std::string var_name = _phases[i];
      _problem->addAuxVariable(aux_var_type, var_name, params);
    }
    for (unsigned int j = 0; j < _species.size(); j++)
    {
      std::string var_name = _species[j];
      _problem->addAuxVariable(aux_var_type, var_name, params);
    }
    for (unsigned int i = 0; i < _element_potentials.size(); i++)
    {
      std::string var_name = _element_potentials[i];
      _problem->addAuxVariable(aux_var_type, var_name, params);
    }
  }
  else if (_current_task == "add_ic" && _inital_values_file_name != "none")
  {
    readCSV();
    for (auto it : _initial_conditions)
    {
      std::string class_name = "ConstantIC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = it.first;
      params.set<Real>("value") = it.second;
      _problem->addInitialCondition(class_name, it.first + "_ic", params);
    }
  }
  else if (_current_task == "add_user_object")
  {
    //
    // Initiate Chemistry Model
    //
    if (_thermoFile != "none")
    {
      char cThermoFileName[120];
      int idbg = 0;
      Thermochimica::ConvertToFortran(cThermoFileName, sizeof cThermoFileName, _thermoFile.c_str());
      FORTRAN_CALL(Thermochimica::setthermofilename)(cThermoFileName);
      //    Moose::out << "CUO initialize " << _thermoFile << "\n";
      // Read in thermodynamics model, only once
      FORTRAN_CALL(Thermochimica::ssparsecsdatafile)();
      FORTRAN_CALL(Thermochimica::checkinfothermo)(&idbg);
      if (idbg != 0)
      {
        mooseError("Thermochimica data file cannot be parsed.");
      }

      Thermochimica::checkTemperature(_tunit);
      Thermochimica::checkPressure(_punit);
      Thermochimica::checkMass(_munit);

      // Translate string to fortran string
      char cTFtn[15];
      Thermochimica::ConvertToFortran(cTFtn, sizeof cTFtn, _tunit.c_str());
      char cPFtn[15];
      Thermochimica::ConvertToFortran(cPFtn, sizeof cPFtn, _punit.c_str());
      char cMFtn[15];
      Thermochimica::ConvertToFortran(cMFtn, sizeof cMFtn, _munit.c_str());
      FORTRAN_CALL(Thermochimica::setunittemperature)(cTFtn);
      FORTRAN_CALL(Thermochimica::setunitpressure)(cPFtn);
      FORTRAN_CALL(Thermochimica::setunitmass)(cMFtn);
    }
  }

  else if (_current_task == "add_aux_kernel" && _var_type == "aux")
  {
    InputParameters params = _factory.getValidParams("SelfAux");
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    for (unsigned int i = 0; i < _phases.size(); i++)
    {
      std::string ker_name = _phases[i];
      params.set<AuxVariableName>("variable") = ker_name;
      _problem->addAuxKernel("SelfAux", ker_name, params);
    }
    for (unsigned int j = 0; j < _species.size(); j++)
    {
      std::string ker_name = _species[j];
      params.set<AuxVariableName>("variable") = ker_name;
      _problem->addAuxKernel("SelfAux", ker_name, params);
    }
    for (unsigned int i = 0; i < _element_potentials.size(); i++)
    {
      std::string ker_name = _element_potentials[i];
      params.set<AuxVariableName>("variable") = ker_name;
      _problem->addAuxKernel("SelfAux", ker_name, params);
    }
  }

#endif
}

void
ChemicalCompositionAction::readCSV()
{
  std::ifstream file(_inital_values_file_name.c_str());
  if (!file.good())
    mooseError("In ChemicalCompositionAction ",
               _name,
               ": Error opening file '" + _inital_values_file_name + "'.");

  unsigned int n_lines = 0;
  std::string line;
  while (getline(file, line))
  {
    // Replace all commas with spaces
    while (size_t pos = line.find(','))
    {
      if (pos == line.npos)
        break;
      line.replace(pos, 1, 1, ' ');
    }

    if (n_lines > 0)
    {
      std::istringstream iss(line);

      std::string el_name;
      iss >> el_name;
      Real f;
      iss >> f;

      _initial_conditions[el_name] = f;
    }
    n_lines++;
  }
}
