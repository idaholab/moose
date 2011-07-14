// Copyright(C) 1999-2010
// Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
// certain rights in this software.
//         
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Sandia Corporation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "FileInfo.h"

#include <string>

#if defined(__PUMAGON__)
#define NPOS (size_t)-1
#else
#define NPOS std::string::npos
#endif

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>

namespace {
  bool internal_access(const std::string &name, int mode);
  bool do_stat(const std::string &my_filename, struct stat *s);
}

FileInfo::FileInfo()
  : filename_(""), exists_(false), readable_(false) {}
  
FileInfo::FileInfo(const std::string &my_filename)
  : filename_(my_filename), exists_(false), readable_(false)
{
  exists_   = internal_access(filename_, F_OK);
  readable_ = internal_access(filename_, R_OK);
}

FileInfo::FileInfo(const char   *my_filename)
  : filename_(std::string(my_filename)), exists_(false), readable_(false)
{
  exists_   = internal_access(filename_, F_OK);
  readable_ = internal_access(filename_, R_OK);
}

FileInfo::FileInfo(const FileInfo& copy_from)
  : filename_(copy_from.filename_), exists_(copy_from.exists_),
    readable_(copy_from.readable_) {}

FileInfo::FileInfo(const std::string &dirpath, const std::string &my_filename)
  : filename_("")
{
  static std::string SLASH("/");

  if (!dirpath.empty()) {
    filename_ = dirpath;
    if (filename_.at(filename_.size()-1) != '/')
      filename_ += SLASH;
  }
  filename_ += my_filename;
  exists_   = internal_access(filename_, F_OK);
  readable_ = internal_access(filename_, R_OK);
}

FileInfo::~FileInfo() {}

//: Returns TRUE if the file exists (is readable)
bool FileInfo::exists()      const
{
  return exists_;
}

//: Returns TRUE if the file is readable
bool FileInfo::is_readable() const
{
  return readable_;
}

//: Returns TRUE if the file is writable
bool FileInfo::is_writable() const
{
  return internal_access(filename_, W_OK);
}

//: Returns TRUE if the file is executable
bool FileInfo::is_executable() const
{
  return internal_access(filename_, X_OK);
}

//: Returns TRUE if we are pointing to a file or a symbolic link to
//: a file.
bool FileInfo::is_file()     const
{
  struct stat s;
  if (do_stat(filename_.c_str(), &s))
    return S_ISREG(s.st_mode);
  else
    return false;
}

//: Returns TRUE if we are pointing to a directory or a symbolic link to
//: a directory.
bool FileInfo::is_dir()      const
{
  struct stat s;
  if (do_stat(filename_.c_str(), &s))
    return S_ISDIR(s.st_mode);
  else
    return false;
}

//: Returns TRUE if we are pointing to a symbolic link
bool FileInfo::is_symlink()  const
{
  struct stat s;
  if (lstat(filename_.c_str(), &s) == 0)
    return S_ISLNK(s.st_mode);
  else
    return false;
}

//: Time of last data modification. See 'man stat(2)'
time_t FileInfo::modified() const
{
  struct stat s;
  if (do_stat(filename_.c_str(), &s))
    return s.st_mtime;
  else
    return 0;
}

//: Time of last access
time_t FileInfo::accessed() const
{
  struct stat s;
  if (do_stat(filename_.c_str(), &s))
    return s.st_atime;
  else
    return 0;
}

//: Time of last status change. (creation, chmod, ...)
time_t FileInfo::created() const
{
  struct stat s;
  if (do_stat(filename_.c_str(), &s))
    return s.st_ctime;
  else
    return 0;
}

//: File size in bytes. Only if is_file() == true
off_t  FileInfo::size() const
{
  struct stat s;
  if (do_stat(filename_.c_str(), &s))
    return s.st_size;
  else
    return 0;
}

//: Returns the filename
const std::string FileInfo::filename()  const
{
  return filename_;
}

//: Sets the filename
void FileInfo::set_filename(const std::string &name)
{
  filename_ = name;
  exists_   = internal_access(filename_, F_OK);
  readable_ = internal_access(filename_, R_OK);
}

//: Sets the filename
void FileInfo::set_filename(const char *name)
{
  filename_ = std::string(name);
  exists_   = internal_access(filename_, F_OK);
  readable_ = internal_access(filename_, R_OK);
}

//: Returns the filename extension or the empty string if there is
//: no extension.  Assumes extension is all characters following the
//: last period.
const std::string FileInfo::extension() const
{
  size_t ind  = filename_.find_last_of(".", NPOS);
  size_t inds = filename_.find_last_of("/", NPOS);

  // Protect against './filename' returning /filename as extension
  if (ind != NPOS && (inds == NPOS || inds < ind))
    return filename_.substr(ind+1, filename_.size());
  else
    return std::string();
}

const std::string FileInfo::pathname() const
{
  size_t ind = filename_.find_last_of("/", filename_.size());
  if (ind != NPOS)
    return filename_.substr(0,ind);
  else
    return std::string();
}

const std::string FileInfo::tailname() const
{
  size_t ind = filename_.find_last_of("/", filename_.size());
  if (ind != NPOS)
    return filename_.substr(ind+1, filename_.size());
  else
    return filename_; // No path, just return the filename
}

const std::string FileInfo::basename() const
{
  std::string tail = tailname();

  // Strip off the extension
  size_t ind = tail.find_last_of('.', tail.size());
  if (ind != NPOS)
    return tail.substr(0,ind);
  else
    return tail;
}

const std::string FileInfo::realpath() const
{
  char *path = ::realpath(filename_.c_str(), NULL);
  if (path)
    return std::string(path);
  else
    return filename_;
}

bool FileInfo::remove_file()
{
  int success = std::remove(filename_.c_str());
  return success == 0;
}

namespace {
  bool internal_access(const std::string& name, int mode)
  {
    if (name.empty())
      return false;
    if (::access(name.c_str(), mode) != 0)
      return false;
    return true;
  }

  bool do_stat(const std::string &filename, struct stat *s)
  {
#if defined(__PUMAGON__)
    // Portland pgCC compiler on janus has 'char*' instead of 'const char*' for
    // first argument to stat function.
    return (stat((char*)filename.c_str(), s) == 0);
#else
    return (stat(filename.c_str(), s) == 0);
#endif
  }
}
