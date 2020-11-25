//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "ConsoleStream.h"
#include "MooseUtils.h"
#include "OutputWarehouse.h"

std::mutex _stream_mutex;

ConsoleStream::ConsoleStream(OutputWarehouse & output_warehouse)
  : _output_warehouse(output_warehouse), _oss(std::make_shared<std::ostringstream>())
{
}

static std::mutex manip_mutex;

const ConsoleStream &
ConsoleStream::operator<<(const StandardEndLine & manip) const
{
  const std::lock_guard<std::mutex> lock(manip_mutex);

  if (manip == (std::basic_ostream<char> & (*)(std::basic_ostream<char> &)) & std::endl)
    (*_oss) << '\n';
  else
    (*_oss) << manip;

  _output_warehouse.mooseConsole(*_oss);

  return *this;
}

void
ConsoleStream::unsetf(std::ios_base::fmtflags mask) const
{
  _oss->unsetf(mask);
}

std::streamsize
ConsoleStream::precision() const
{
  return _oss->precision();
}

std::streamsize
ConsoleStream::precision(std::streamsize new_precision) const
{
  return _oss->precision(new_precision);
}

std::ios_base::fmtflags
ConsoleStream::flags() const
{
  return _oss->flags();
}

std::ios_base::fmtflags
ConsoleStream::flags(std::ios_base::fmtflags new_flags) const
{
  return _oss->flags(new_flags);
}

unsigned long long int
ConsoleStream::numPrinted() const
{
  return _output_warehouse.numPrinted();
}
