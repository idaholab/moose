//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "XMLOutput.h"
#include "FEProblem.h"
#include "MooseApp.h"

registerMooseObject("MooseApp", XMLOutput);

InputParameters
XMLOutput::validParams()
{
  InputParameters params = AdvancedOutput::validParams();
  params.addClassDescription("Output for VectorPostprocessor using XML format.");
  params += AdvancedOutput::enableOutputTypes("vector_postprocessor");
  return params;
}

XMLOutput::XMLOutput(const InputParameters & parameters) : AdvancedOutput(parameters)
{
  // Creates section in VPP
  _xml_doc.append_child("VectorPostprocessors");
}

std::string
XMLOutput::filename()
{
  if (processor_id() > 0)
  {
    std::ostringstream file_name;
    int digits = MooseUtils::numDigits(n_processors());
    file_name << _file_base << ".xml"
              << "." << std::setw(digits) << std::setfill('0') << processor_id();
    return file_name.str();
  }
  return _file_base + ".xml";
}

void
XMLOutput::outputVectorPostprocessors()
{
  // Create pugi node for storing vector data
  auto vpp_node = _xml_doc.child("VectorPostprocessors");
  auto vec_node = vpp_node.append_child("Vectors");

  // Populate output information from FEProblem, do not use AdvancedOutput because the psuedo time
  // is not required.
  vec_node.append_attribute("time") = _problem_ptr->time();
  vec_node.append_attribute("timestep") = _problem_ptr->timeStep();
  if (_execute_enum.contains(EXEC_LINEAR) && !_on_nonlinear_residual)
    vec_node.append_attribute("linear_iteration") = _linear_iter;
  if (_execute_enum.contains(EXEC_NONLINEAR))
    vec_node.append_attribute("nonlinear_iteration") = _nonlinear_iter;

  // VPP data object for determining if vectors are distributed
  const auto & vpp_data = _problem_ptr->getVectorPostprocessorData();

  // Loop through the VPP objects that should be output
  const std::set<std::string> & out = getVectorPostprocessorOutput();
  for (const auto & vpp_name : out)
  {
    if (_problem_ptr->vectorPostprocessorHasVectors(vpp_name))
    {
      // Loop through the vectors to be output
      const auto & vectors = _problem_ptr->getVectorPostprocessorVectors(vpp_name);
      for (const auto & vec_it : vectors)
      {
        const bool distributed = vpp_data.isDistributed(vpp_name);
        if (processor_id() == 0 || distributed)
        {
          // Create a Vector node and associated operators
          auto data_node = vec_node.append_child("Vector");
          data_node.append_attribute("object") = vpp_name.c_str();
          data_node.append_attribute("name") = vec_it.first.c_str();
          data_node.append_attribute("distributed") = distributed;
          if (distributed)
          {
            data_node.append_attribute("processor_id") = processor_id();
            data_node.append_attribute("n_processors") = n_processors();
            _distributed = true;
          }

          // Write the vector of data
          const auto & vector = *vec_it.second.current;
          std::ostringstream oss;
          std::copy(vector.begin(), vector.end(), infix_ostream_iterator<Real>(oss, " "));
          data_node.text().set(oss.str().c_str());
        }
      }
    }
  }
}

void
XMLOutput::output(const ExecFlagType & type)
{
  AdvancedOutput::output(type);
  if (processor_id() == 0 || _distributed)
    _xml_doc.save_file(filename().c_str());
}
