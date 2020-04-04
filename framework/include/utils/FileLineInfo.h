//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>
#include <map>

/**
 * Holds file and line information.
 */
class FileLineInfo
{
public:
  FileLineInfo();
  FileLineInfo(const std::string & f, int l);
  /**
   * @return Whether this holds valid file line information.
   */
  bool isValid() const;
  int line() const;
  std::string file() const;

protected:
  int _line;
  std::string _file;
};

/**
 * A mapping between a series of keys to a FileLineInfo.
 * This is intended to replace having a std::pair or a std::tuple
 * as a key to a map holding the FileLineInfo.
 */
class FileLineInfoMap
{
public:
  /**
   * Associate a key with file/line info
   * @param key0 Key
   * @param file file
   * @param line line number
   */
  void addInfo(const std::string & key0, const std::string & file, int line);

  /**
   * Associate a key with file/line info
   * @param key0 Key
   * @param key1 Key
   * @param file file
   * @param line line number
   */
  void
  addInfo(const std::string & key0, const std::string & key1, const std::string & file, int line);

  /**
   * Associate a key with file/line info
   * @param key0 Key
   * @param key1 Key
   * @param key2 Key
   * @param file file
   * @param line line number
   */
  void addInfo(const std::string & key0,
               const std::string & key1,
               const std::string & key2,
               const std::string & file,
               int line);

  /**
   * Get file/line info for a key
   * @param key0 Key
   * @return FileLineInfo
   */
  FileLineInfo getInfo(const std::string & key0) const;

  /**
   * Get file/line info for a pair of keys.
   * @param key0 Key
   * @param key1 Key
   * @return FileLineInfo
   */
  FileLineInfo getInfo(const std::string & key0, const std::string & key1) const;

  /**
   * Get file/line info for a pair of keys.
   * @param key0 Key
   * @param key1 Key
   * @param key2 Key
   * @return FileLineInfo
   */
  FileLineInfo
  getInfo(const std::string & key0, const std::string & key1, const std::string & key2) const;

protected:
  /**
   * Makes a unique key for the map given two strings
   */
  std::string makeKey(const std::string & key0, const std::string & key1) const;

  /**
   * Makes a unique key given three strings
   */
  std::string
  makeKey(const std::string & key0, const std::string & key1, const std::string & key2) const;
  std::map<std::string, FileLineInfo> _map;
};
