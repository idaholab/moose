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
#include "tecplot_io.h"
#include "o_string_stream.h"

TecplotOutput::TecplotOutput(EquationSystems & es, bool binary) :
    Outputter(es),
    _file_num(0),
    _binary(binary)
{
}

TecplotOutput::~TecplotOutput()
{
}

std::string
TecplotOutput::getFileName(const std::string & file_base)
{
  OStringStream stream_file_base;

  stream_file_base << file_base << "_";
  OSSRealzeroright(stream_file_base, 4, 0, _file_num);

  return stream_file_base.str();
}


void
TecplotOutput::output(const std::string & file_base, Real /*time*/)
{
  TecplotIO out(_es.get_mesh(), _binary);

  out.write_equation_systems(getFileName(file_base) + ".plt", _es);
  _file_num++;
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
