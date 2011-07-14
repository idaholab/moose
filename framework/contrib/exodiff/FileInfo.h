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

#ifndef FileInfo_h
#define FileInfo_h

#include <string>
#include <iosfwd>

#include <string>
#include <sys/types.h>

  /*! \class FileInfo 
   *  \author Greg Sjaardema
   *  \brief  Return information about the specified file.
   *
   *  A very minimal class (at least it used to be) for providing
   *  information about a file.  FileInfo provides information about a
   *  file's name, path, and type (directory, symbolic link, file).  Other
   *  information could be added as needed.  It currently does not cache
   *  any information, so if it is heavily used, a caching capability
   *  should be added.  See the Qt Toolkit QFileInfo class for a richer
   *  file class.
   */

class FileInfo
{
 public:
  //! Empty class referring to no file.
  FileInfo();
  
  //! Create object referring to file with name \a filename
  //! \param filename name of file
  explicit FileInfo(const std::string &filename);

  //! Create object referring to file with name \a filename
  //! \param filename name of file
  explicit FileInfo(const char   *filename);

  //! Copy constructor
  FileInfo(const FileInfo&);

  //! Constructor
  //! \param dirpath Directory Path
  //! \param filename base filename
  FileInfo(const std::string &dirpath, const std::string &filename);
  
  ~FileInfo();

  bool exists()        const; //!< returns True if file exists, false if nonexistant
  bool is_readable()   const; //!< Exists and is readable
  bool is_writable()   const; //!< Exists and is writable
  bool is_executable() const; //!< Exists and is executable

  bool is_file()       const; //!< Is a plain file
  bool is_dir()        const; //!< Is a directory
  bool is_symlink()    const; //!< Is a symbolic link to a file or directory
  
  time_t modified()    const; //!< Time of last data modification. See 'man stat(2)'
  time_t accessed()    const; //!< Time of last access
  time_t created()     const; //!< Time of last status change. (creation, chmod, ...)
  
  off_t  size()        const; //!< File size in bytes. Only if is_file() == true
  
  const std::string filename()  const; //!< Complete filename including path
  const std::string basename()  const; //!< strip path and extension
  const std::string tailname()  const; //!< basename() + extension()
  const std::string extension() const; //!< file extension.
  const std::string pathname()  const; //!< directory path, no filename
  const std::string realpath()  const; //!< canonicalized absolute path

  void  set_filename(const std::string &name);
  void  set_filename(const char *name);
  
  bool  operator==(const FileInfo &other) const
  { return filename_ == other.filename_; }
  
  bool  operator!=(const FileInfo &other) const
  { return filename_ != other.filename_; }
  
  bool remove_file();
    
 private:
  std::string filename_;
  bool exists_;   //<! this is used frequently, check on creation
  bool readable_; //<! this is used frequently, check on creation
};
#endif 

