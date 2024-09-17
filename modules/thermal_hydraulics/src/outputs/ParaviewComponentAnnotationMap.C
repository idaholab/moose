//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParaviewComponentAnnotationMap.h"

#include "nlohmann/json.h"
#include "LockFile.h"
#include "JsonIO.h"
#include "THMProblem.h"
#include "Component.h"
#include <fstream>

registerMooseObject("ThermalHydraulicsApp", ParaviewComponentAnnotationMap);

/// KAAMS color profile from paraview
static const std::vector<Real> kaams_colors = {1.0, 1.0, 1.0,  1.0,  0.0,  0.0, 0.0,  1.0,  0.0,
                                               0.0, 0.0, 1.0,  1.0,  1.0,  0.0, 1.0,  0.0,  1.0,
                                               0.0, 1.0, 1.0,  0.63, 0.63, 1.0, 0.67, 0.5,  0.33,
                                               1.0, 0.5, 0.75, 0.53, 0.35, 0.7, 1.0,  0.75, 0.5};

InputParameters
ParaviewComponentAnnotationMap::validParams()
{
  InputParameters params = FileOutput::validParams();
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL};
  params.set<ExecFlagEnum>("execute_on") = execute_options;
  return params;
}

ParaviewComponentAnnotationMap::ParaviewComponentAnnotationMap(const InputParameters & parameters)
  : FileOutput(parameters)
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem_ptr);
  if (!thm_problem)
    mooseError(name(),
               ": ParaviewComponentAnnotationMap can only be used with THM-based simulation.");
}

void
ParaviewComponentAnnotationMap::output()
{
  if (processor_id() == 0)
  {
    THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem_ptr);
    const std::vector<std::shared_ptr<Component>> & comps = thm_problem->getComponents();

    std::vector<Real> clrs;
    unsigned int clr_idx = 0;

    std::vector<std::string> anns;
    for (auto & comp : comps)
    {
      const auto & subdomains = comp->getSubdomainNames();
      for (auto & sn : subdomains)
      {
        SubdomainID sid = _mesh_ptr->getSubdomainID(sn);

        anns.push_back(Moose::stringify(sid));
        anns.push_back(sn);

        clrs.push_back(kaams_colors[clr_idx++]);
        clrs.push_back(kaams_colors[clr_idx++]);
        clrs.push_back(kaams_colors[clr_idx++]);
        // 12 colors with RGB values
        clr_idx = clr_idx % (12 * 3);
      }
    }

    nlohmann::json json;
    json[0]["Annotations"] = anns;
    json[0]["IndexedColors"] = clrs;
    json[0]["Name"] = _file_base;

    std::ofstream out(filename().c_str());
    out << std::setw(4) << json << std::endl;
  }
}

std::string
ParaviewComponentAnnotationMap::filename()
{
  return _file_base + ".json";
}
