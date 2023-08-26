//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StringInputStream.h"

#include "MooseError.h"

StringInputStream::StringInputStream(std::unique_ptr<std::stringstream> stream)
  : InputStream(), _stream(std::move(stream))
{
}

std::shared_ptr<std::istream>
StringInputStream::get() const
{
  mooseAssert(_stream, "Not valid");

  std::shared_ptr<std::istream> stream = std::make_unique<std::istream>(_stream->rdbuf());
  addSharedStream(stream);
  return stream;
}

std::unique_ptr<std::stringstream>
StringInputStream::release()
{
  if (inUse())
    mooseError("StringInputStream::release(): Cannot release; still in use");
  return std::move(_stream);
}
