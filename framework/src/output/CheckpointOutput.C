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

#include "CheckpointOutput.h"
#include "Problem.h"
#include "FEProblem.h"

#include "libmesh/checkpoint_io.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/parallel_mesh.h"

#include <sstream>
#include <iomanip>

CheckpointOutput::CheckpointOutput(EquationSystems & es, bool binary, SubProblem & sub_problem) :
    Outputter(es, sub_problem, "CheckpointOutput"),
    _binary(binary),
    _fe_problem(dynamic_cast<FEProblem *>(&sub_problem))
{
}

CheckpointOutput::~CheckpointOutput()
{
}

std::string
CheckpointOutput::getFileName(const std::string & file_base, unsigned int t_step)
{
  std::ostringstream stream_file_base;

  stream_file_base << file_base
                   << std::setw(4)
                   << std::setprecision(0)
                   << std::setfill('0')
                   << std::right
                   << t_step;

  return stream_file_base.str();
}


void
CheckpointOutput::output(const std::string & file_base, Real /*time*/, unsigned int t_step)
{
  MeshBase & mesh = _es.get_mesh();

  CheckpointIO io(mesh, _binary);

  bool renumber = true;

  if(_fe_problem && !_fe_problem->adaptivity().isOn())
    renumber = false;

  if (_binary)
  {
    io.write(getFileName(file_base, t_step)+"_mesh.cpr");
    _es.write (getFileName(file_base, t_step)+".xdr", ENCODE, EquationSystems::WRITE_DATA | EquationSystems::WRITE_ADDITIONAL_DATA | EquationSystems::WRITE_PARALLEL_FILES, renumber);
  }
  else
  {
    io.write(getFileName(file_base, t_step)+"_mesh.cpa");
    _es.write (getFileName(file_base, t_step)+".xda", WRITE, EquationSystems::WRITE_DATA | EquationSystems::WRITE_ADDITIONAL_DATA | EquationSystems::WRITE_PARALLEL_FILES, renumber);
  }
}

void
CheckpointOutput::outputPps(const std::string & /*file_base*/, const FormattedTable & /*table*/, Real /*time*/)
{
  // XDA does not support PPS values
}


void
CheckpointOutput::meshChanged()
{
}

void
CheckpointOutput::sequence(bool /*state*/)
{
  // do nothing, XDA files are always sequences
}
