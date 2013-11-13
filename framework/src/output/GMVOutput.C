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

#include "GMVOutput.h"
#include "Problem.h"

// libMesh
#include "libmesh/gmv_io.h"

// C++
#include <sstream>
#include <iomanip>

GMVOutput::GMVOutput(EquationSystems & es) :
    Outputter(es)
{
}

GMVOutput::~GMVOutput()
{
}

std::string
GMVOutput::getFileName(const std::string & file_base, unsigned int t_step)
{
  std::ostringstream stream_file_base;

  stream_file_base << file_base
                   << "_"
                   << std::setw(4)
                   << std::setprecision(0)
                   << std::setfill('0')
                   << std::right
                   << t_step;

  return stream_file_base.str() + ".gmv";
}


void
GMVOutput::output(const std::string & file_base, Real /*time*/, unsigned int t_step)
{
  GMVIO out(_es.get_mesh());

  out.write_equation_systems(getFileName(file_base, t_step), _es);
}

void
GMVOutput::outputPps(const std::string & /*file_base*/, const FormattedTable & /*table*/, Real /*time*/)
{
  // GMV does not support PPS values
}


void
GMVOutput::meshChanged()
{
}

void
GMVOutput::sequence(bool /*state*/)
{
  // do nothing, GMV files are always sequences
}
