//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileRangeBuilder.h"

// MOOSE includes
#include "InputParameters.h"

#include "libmesh/int_range.h"
#include "pcrecpp.h"
#include "tinydir.h"

InputParameters
FileRangeBuilder::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<FileName>("file",
                            "Name of single image file to extract mesh parameters from.  "
                            "If provided, a 2D mesh is created.");
  params.addParam<FileNameNoExtension>("file_base",
                                       "Image file base to open, use this option when "
                                       "a stack of images must be read (ignored if "
                                       "'file' is given)");
  params.addParam<std::vector<unsigned int>>(
      "file_range",
      "Range of images to analyze, used with 'file_base' (ignored if 'file' is given)");
  params.addParam<std::string>("file_suffix", "Suffix of the file to open, e.g. 'png'");
  return params;
}

FileRangeBuilder::FileRangeBuilder(const InputParameters & params) : _status(0)
{
  bool has_file = params.isParamValid("file"), has_file_base = params.isParamValid("file_base"),
       has_file_range = params.isParamValid("file_range"),
       has_file_suffix = params.isParamValid("file_suffix");

  // Variables to be (possibly) used below...
  std::string file;
  std::string file_base;
  std::vector<unsigned int> file_range;

  if (has_file)
  {
    file = params.get<FileName>("file");

    // Set the file_suffix parameter in the passed-in params object based on the input filename
    _file_suffix = file.substr(file.find_last_of(".") + 1);
  }
  if (has_file_base)
    file_base = params.get<FileNameNoExtension>("file_base");
  if (has_file_range)
    file_range = params.get<std::vector<unsigned int>>("file_range");
  if (has_file_suffix)
    _file_suffix = params.get<std::string>("file_suffix");

  // Check that the combination of params provided is valid

  // 1.) Provided both a filename and a file base
  if (has_file && has_file_base)
  {
    _status = 1;
    return;
  }

  // 2.) Provided neither a filename nor a file base
  if (!has_file && !has_file_base)
  {
    _status = 2;
    return;
  }

  // 3.) Provided a file base but not a suffix
  if (has_file_base && !has_file_suffix)
  {
    _status = 3;
    return;
  }

  // 4.) Provided a filename and a range, warn that the range will be ignored.
  if (has_file && has_file_range)
    mooseWarning("Warning: file_range was ignored since a filename was provided.");

  // 5.) Provided a file_base but not a file_range, we'll create one
  if (has_file_base && !has_file_range)
  {
    file_range.push_back(0);
    file_range.push_back(std::numeric_limits<unsigned int>::max());
  }

  // 6.) Provided a file_range with a single entry, so repeat it for
  // them.  This signifies a single image (i.e. 2D Mesh) is to be
  // used.
  if (has_file_range && file_range.size() == 1)
    file_range.push_back(file_range[0]);

  // 7.) Provided a file_range with too many entries, print a
  // warning and truncate the extra values.
  if (has_file_range && file_range.size() != 2)
  {
    mooseWarning("A maximum of two values are allowed in the file_range, extra values truncated.");
    file_range.resize(2);
  }

  // Make sure that the range is in ascending order
  std::sort(file_range.begin(), file_range.end());

  // Build up the filenames parameter, and inject it into the InputParameters object
  if (has_file)
    _filenames.push_back(file);

  else if (has_file_base)
  {
    // Separate the file base from the path
    std::pair<std::string, std::string> split_file = MooseUtils::splitFileName(file_base);

    // Create directory object
    tinydir_dir dir;
    tinydir_open_sorted(&dir, split_file.first.c_str());

    // This regex will capture the file_base and any number of digits when used with FullMatch()
    std::ostringstream oss;
    oss << "(" << split_file.second << ".*?(\\d+))\\..*";
    pcrecpp::RE file_base_and_num_regex(oss.str());

    // Loop through the files in the directory
    for (const auto i : make_range(dir.n_files))
    {
      // Update the current file
      tinydir_file file;
      tinydir_readfile_n(&dir, &file, i);

      // Store the file if it has proper extension as in numeric range
      if (!file.is_dir && MooseUtils::hasExtension(file.name, _file_suffix))
      {
        std::string the_base;
        unsigned int file_num = 0;
        file_base_and_num_regex.FullMatch(file.name, &the_base, &file_num);

        if (!the_base.empty() && file_num >= file_range[0] && file_num <= file_range[1])
          _filenames.push_back(split_file.first + "/" + file.name);
      }
    }
    tinydir_close(&dir);
  }

  else
    mooseError("We'll never get here!");

  // If we made it here, there were no errors
}

void
FileRangeBuilder::errorCheck()
{
  switch (_status)
  {
    case 0:
      return;
    case 1:
      mooseError("Cannot provide both file and file_base parameters");
      break;
    case 2:
      mooseError("You must provide a valid value for either the 'file' parameter or the "
                 "'file_base' parameter.");
      break;
    case 3:
      mooseError(
          "If you provide a 'file_base', you must also provide a valid 'file_suffix', e.g. 'png'.");
      break;
    default:
      mooseError("Unknown error code!");
  }
}
