//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileInputStream.h"

#include "MooseError.h"

#include <fstream>

FileInputStream::FileInputStream(const std::string & filename) : InputStream(), _filename(filename)
{
}

std::shared_ptr<std::istream>
FileInputStream::get() const
{
  std::shared_ptr<std::istream> stream = std::make_unique<std::ifstream>();
  addSharedStream(stream);

  auto & in = *static_cast<std::ifstream *>(stream.get());
  in.open(_filename.c_str(), std::ios::in | std::ios::binary);
  if (in.fail())
    mooseError("Unable to open file ", std::filesystem::absolute(_filename));

  return stream;
}

std::optional<std::filesystem::path>
FileInputStream::getFilename() const
{
  return _filename;
}
