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

#include "VTKOutput.h"
#include "Problem.h"

// libMesh
#include "libmesh/vtk_io.h"

// C++
#include <sstream>
#include <iomanip>

VTKOutput::VTKOutput(EquationSystems & es) :
    Outputter(es),
    _file_num(0)
{
}

VTKOutput::~VTKOutput()
{
}

std::string
VTKOutput::getFileName(const std::string & file_base)
{
  std::ostringstream stream_file_base;

  stream_file_base << file_base
                   << "_"
                   << std::setw(4)
                   << std::setprecision(0)
                   << std::setfill('0')
                   << std::right
                   << _file_num;

  return stream_file_base.str() + ".vtk";
}


void
VTKOutput::output(const std::string & file_base, Real /*time*/)
{
  VTKIO out(_es.get_mesh());

  out.write_equation_systems(getFileName(file_base), _es);
  _file_num++;
}

void
VTKOutput::outputPps(const std::string & /*file_base*/, const FormattedTable & /*table*/, Real /*time*/)
{
  // VTK does not support PPS values
}


void
VTKOutput::meshChanged()
{
}

void
VTKOutput::sequence(bool /*state*/)
{
  // do nothing, VTK files are always sequences
}
