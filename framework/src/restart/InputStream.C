//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InputStream.h"

#include "MooseError.h"

InputStream::~InputStream()
{
  if (inUse())
    mooseError("Stream still in use");
}

void
InputStream::addSharedStream(std::shared_ptr<std::istream> stream) const
{
  _shared_streams.push_back(stream);
}

bool
InputStream::inUse() const
{
  for (const auto & stream : _shared_streams)
    if (stream.use_count() > 1)
      return true;
  return false;
}

std::optional<std::filesystem::path>
InputStream::getFilename() const
{
  return {};
}
