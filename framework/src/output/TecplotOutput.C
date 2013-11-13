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

#include "TecplotOutput.h"
#include "Problem.h"
// libMesh
#include "libmesh/tecplot_io.h"

// C++
#include <sstream>
#include <iomanip>

TecplotOutput::TecplotOutput(EquationSystems & es, bool binary) :
    Outputter(es),
    _binary(binary)
{
}

TecplotOutput::~TecplotOutput()
{
}

std::string
TecplotOutput::getFileName(const std::string & file_base, unsigned int t_step)
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
TecplotOutput::output(const std::string & file_base, Real /*time*/, unsigned int t_step)
{
  TecplotIO out(_es.get_mesh(), _binary);

  out.write_equation_systems(getFileName(file_base, t_step) + ".plt", _es);
}

void
TecplotOutput::outputPps(const std::string & /*file_base*/, const FormattedTable & /*table*/, Real /*time*/)
{
  // Tecplot does not support PPS values
}


void
TecplotOutput::meshChanged()
{
}

void
TecplotOutput::sequence(bool /*state*/)
{
  // do nothing, Tecplot files are always sequences
}
