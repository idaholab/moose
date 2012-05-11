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

#include "Resurrector.h"
#include "FEProblem.h"
#include <stdio.h>

Resurrector::Resurrector(FEProblem & fe_problem) :
    _fe_problem(fe_problem),
    _restart(false),
    _num_restart_files(0),
    _xda(_fe_problem.es())
{
}

Resurrector::~Resurrector()
{
}

void
Resurrector::setRestartFile(const std::string & file_base)
{
  _restart = true;
  _restart_file_base = file_base;
}

void
Resurrector::restartFromFile()
{
  std::string file_name(_restart_file_base + ".xda");
  _fe_problem._eq.read(file_name, libMeshEnums::READ, EquationSystems::READ_DATA);
  _fe_problem._nl.update();
}

void
Resurrector::setNumRestartFiles(unsigned int num_files)
{
  _num_restart_files = num_files;
}

void
Resurrector::write()
{
  if (_num_restart_files == 0)
    return;

  std::string file_base = _xda.getFileName(_fe_problem.out().fileBase() + "_restart");
  _restart_file_names.push_back(file_base);

  _xda.output(file_base, _fe_problem.time());                   // time does not have any effect here actually

  if (_restart_file_names.size() > _num_restart_files)
  {
    // remove the head from the list
    std::string fb = _restart_file_names.front();
    _restart_file_names.pop_front();
    // and delete the file
    std::string fn;
    fn = fb + "_mesh.xda";
    remove(fn.c_str());           // mesh
    fn = fb + ".xda";
    remove(fn.c_str());           // solution
  }
}
