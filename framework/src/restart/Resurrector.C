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
#include "MooseUtils.h"

#include <stdio.h>
#include <sys/stat.h>

const std::string Resurrector::MAT_PROP_EXT(".msmp");
const std::string Resurrector::RESTARTABLE_DATA_EXT(".rd");

Resurrector::Resurrector(FEProblem & fe_problem) :
    _fe_problem(fe_problem),
    _num_checkpoint_files(0),
    _checkpoint(_fe_problem.es(), true, fe_problem),
    _mat(_fe_problem),
    _restartable(_fe_problem)
{
}

Resurrector::~Resurrector()
{
}

void
Resurrector::setRestartFile(const std::string & file_base)
{
  _restart_file_base = file_base;
}

void
Resurrector::restartFromFile()
{
  Moose::setup_perf_log.push("restartFromFile()","Resurrector");
  std::string file_name(_restart_file_base + ".xdr");
  MooseUtils::checkFileReadable(file_name);
  _fe_problem._eq.read(file_name, DECODE, EquationSystems::READ_DATA | EquationSystems::READ_ADDITIONAL_DATA, _fe_problem.adaptivity().isOn());
  _fe_problem._nl.update();
  Moose::setup_perf_log.pop("restartFromFile()","Resurrector");
}

void
Resurrector::restartStatefulMaterialProps()
{
  Moose::setup_perf_log.push("restartStatefulMaterialProps()","Resurrector");
  std::string file_name(_restart_file_base + MAT_PROP_EXT);
  _mat.read(file_name);
  Moose::setup_perf_log.pop("restartStatefulMaterialProps()","Resurrector");
}

void
Resurrector::restartRestartableData()
{
  Moose::setup_perf_log.push("restartRestartableData()","Resurrector");
  _restartable.readRestartableData(_restart_file_base + RESTARTABLE_DATA_EXT, _fe_problem._restartable_data, _fe_problem._recoverable_data);
  Moose::setup_perf_log.pop("restartRestartableData()","Resurrector");
}


void
Resurrector::setNumRestartFiles(unsigned int num_files)
{
  _num_checkpoint_files = num_files;
}

void
Resurrector::write()
{
  if (_num_checkpoint_files == 0)
    return;

  Moose::perf_log.push("write()","Resurrector");
  std::string cp_dir = _fe_problem.getCheckpointDir();

  mkdir(cp_dir.c_str(),  S_IRWXU | S_IRGRP);

  std::string s = cp_dir + "/";
  std::string file_base = cp_dir + "/" + _checkpoint.getFileName("", _fe_problem.timeStep());
  _restart_file_names.push_back(file_base);

  _checkpoint.output(s, _fe_problem.time(), _fe_problem.timeStep());                   // time does not have any effect here actually
  if (_fe_problem._material_props.hasStatefulProperties())
    _mat.write(file_base + MAT_PROP_EXT);

  _restartable.writeRestartableData(file_base + RESTARTABLE_DATA_EXT, _fe_problem._restartable_data, _fe_problem._recoverable_data);

  if (_restart_file_names.size() > _num_checkpoint_files)
  {
    unsigned int n_threads = libMesh::n_threads();
    processor_id_type proc_id = libMesh::processor_id();

    // remove the head from the list
    std::string fb = _restart_file_names.front();
    _restart_file_names.pop_front();
    // and delete the file
    std::string fn;
    fn = fb + "_mesh.cpr";
    remove(fn.c_str());           // mesh

    // solution
    {
      std::ostringstream file_name_stream;
      file_name_stream << fb << ".xdr";

      remove(file_name_stream.str().c_str());

      {
        char buf[256];
        std::sprintf(buf, "%04d", proc_id);

        file_name_stream << "." << buf;
      }

      remove(file_name_stream.str().c_str());
    }

    // material properties
    {
      std::ostringstream file_name_stream;
      file_name_stream << fb + MAT_PROP_EXT;
      file_name_stream << "-" << proc_id;

      remove(file_name_stream.str().c_str());
    }

    // user data
    {
      for(unsigned int tid=0; tid<n_threads; tid++)
      {
        std::ostringstream file_name_stream;
        file_name_stream << fb + RESTARTABLE_DATA_EXT;

        file_name_stream << "-" << proc_id;

        if(n_threads > 1)
          file_name_stream << "-" << tid;

        remove(file_name_stream.str().c_str());
      }
    }
  }
  Moose::perf_log.pop("write()","Resurrector");
}

unsigned int
Resurrector::getNumRestartFiles()
{
  return _num_checkpoint_files;
}
