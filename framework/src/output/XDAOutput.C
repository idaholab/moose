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

XDAOutput::XDAOutput(EquationSystems & es) :
    Outputter(es),
    _file_num(0)
{
}

XDAOutput::~XDAOutput()
{
}

std::string
XDAOutput::getFileName(const std::string & file_base)
{
  std::ostringstream stream_file_base;

  stream_file_base << file_base
                   << "_"
                   << std::setw(4)
                   << std::setprecision(0)
                   << std::setfill('0')
                   << std::right
                   << _file_num;

  return stream_file_base.str();
}


void
XDAOutput::output(const std::string & file_base, Real /*time*/)
{
  MeshBase & mesh = _es.get_mesh();

  mesh.write(getFileName(file_base)+"_mesh.xda");
  _es.write (getFileName(file_base)+".xda", libMeshEnums::WRITE);
  _file_num++;
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
