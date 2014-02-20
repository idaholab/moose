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

#include "XDAOutput.h"
#include "Problem.h"

#include <sstream>
#include <iomanip>

XDAOutput::XDAOutput(EquationSystems & es, bool binary, SubProblem & sub_problem) :
    Outputter(es, sub_problem, "XDAOutput"),
    _binary(binary)
{
}

XDAOutput::~XDAOutput()
{
}

std::string
XDAOutput::getFileName(const std::string & file_base, unsigned int t_step)
{
  std::ostringstream stream_file_base;

  stream_file_base << file_base
                   << "_"
                   << std::setw(4)
                   << std::setprecision(0)
                   << std::setfill('0')
                   << std::right
                   << t_step;

  return stream_file_base.str();
}


void
XDAOutput::output(const std::string & file_base, Real /*time*/, unsigned int t_step)
{
  MeshBase & mesh = _es.get_mesh();

  if (_binary)
  {
    mesh.write(getFileName(file_base, t_step)+"_mesh.xdr");
    _es.write (getFileName(file_base, t_step)+".xdr", ENCODE, EquationSystems::WRITE_DATA | EquationSystems::WRITE_ADDITIONAL_DATA);
  }
  else
  {
    mesh.write(getFileName(file_base, t_step)+"_mesh.xda");
    _es.write (getFileName(file_base, t_step)+".xda", WRITE, EquationSystems::WRITE_DATA | EquationSystems::WRITE_ADDITIONAL_DATA);
  }
}

void
XDAOutput::outputPps(const std::string & /*file_base*/, const FormattedTable & /*table*/, Real /*time*/)
{
  // XDA does not support PPS values
}


void
XDAOutput::meshChanged()
{
}

void
XDAOutput::sequence(bool /*state*/)
{
  // do nothing, XDA files are always sequences
}
