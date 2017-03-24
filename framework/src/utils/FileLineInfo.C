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

#include "FileLineInfo.h"

FileLineInfo::FileLineInfo() : _line(-1) {}

FileLineInfo::FileLineInfo(const std::string & f, int l) : _line(l), _file(f) {}

bool
FileLineInfo::isValid() const
{
  return !_file.empty() && _line >= 0;
}

int
FileLineInfo::line() const
{
  return _line;
}

std::string
FileLineInfo::file() const
{
  return _file;
}

void
FileLineInfoMap::addInfo(const std::string & key0, const std::string & file, int line)
{
  FileLineInfo f(file, line);
  if (f.isValid())
    _map[key0] = f;
}

void
FileLineInfoMap::addInfo(const std::string & key0,
                         const std::string & key1,
                         const std::string & file,
                         int line)
{
  addInfo(makeKey(key0, key1), file, line);
}

void
FileLineInfoMap::addInfo(const std::string & key0,
                         const std::string & key1,
                         const std::string & key2,
                         const std::string & file,
                         int line)
{
  addInfo(makeKey(key0, key1, key2), file, line);
}

std::string
FileLineInfoMap::makeKey(const std::string & key0, const std::string & key1) const
{
  return key0 + "%" + key1;
}

std::string
FileLineInfoMap::makeKey(const std::string & key0,
                         const std::string & key1,
                         const std::string & key2) const
{
  return key0 + "%" + key1 + "%" + key2;
}

FileLineInfo
FileLineInfoMap::getInfo(const std::string & key0) const
{
  auto it = _map.find(key0);
  if (it == _map.end())
    return FileLineInfo();
  return it->second;
}

FileLineInfo
FileLineInfoMap::getInfo(const std::string & key0, const std::string & key1) const
{
  return getInfo(makeKey(key0, key1));
}

FileLineInfo
FileLineInfoMap::getInfo(const std::string & key0,
                         const std::string & key1,
                         const std::string & key2) const
{
  return getInfo(makeKey(key0, key1, key2));
}
