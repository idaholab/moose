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
#include "gmv_io.h"
#include "o_string_stream.h"

GMVOutput::GMVOutput(EquationSystems & es) :
    Outputter(es),
    _file_num(0)
{
}

GMVOutput::~GMVOutput()
{
}

std::string
GMVOutput::getFileName(const std::string & file_base)
{
  OStringStream stream_file_base;

  stream_file_base << file_base << "_";
  OSSRealzeroright(stream_file_base, 4, 0, _file_num);

  return stream_file_base.str() + ".gmv";
}


void
GMVOutput::output(const std::string & file_base, Real /*time*/)
{
  GMVIO out(_es.get_mesh());

  out.write_equation_systems(getFileName(file_base), _es);
  _file_num++;
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
