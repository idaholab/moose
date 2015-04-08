/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "FileRangeBuilder.h"
#include "pcrecpp.h"
#include "tinydir.h"

void addFileRangeParams(InputParameters & params)
{
  params.addParam<std::string>("file", "Name of single image file to extract mesh parameters from.  If provided, a 2D mesh is created.");
  params.addParam<std::string>("file_base", "Image file base to open, use this option when a stack of images must be read (ignored if 'file' is given)");
  params.addParam<std::vector<unsigned int> >("file_range", "Range of images to analyze, used with 'file_base' (ignored if 'file' is given)");
  params.addParam<std::string>("file_suffix", "Suffix of the file to open, e.g. 'png'");
}



int parseFileRange(InputParameters & params)
{
  bool
    has_file = params.isParamValid("file"),
    has_file_base = params.isParamValid("file_base"),
    has_file_range = params.isParamValid("file_range"),
    has_file_suffix = params.isParamValid("file_suffix");

  // Variables to be (possibly) used below...
  std::string file;
  std::string file_base;
  std::vector<unsigned int> file_range;
  std::string file_suffix;

  if (has_file)
  {
    file = params.get<std::string>("file");

    // Set the file_suffix parameter in the passed-in params object based on the input filename
    params.set<std::string>("file_suffix") = file.substr(file.find_last_of(".") + 1);
  }
  if (has_file_base)
    file_base = params.get<std::string>("file_base");
  if (has_file_range)
    file_range = params.get<std::vector<unsigned int> >("file_range");
  if (has_file_suffix)
    file_suffix = params.get<std::string>("file_suffix");

  // Check that the combination of params provided is valid

  // 1.) Provided both a filename and a file base
  if (has_file && has_file_base)
    return 1;

  // 2.) Provided neither a filename nor a file base
  if (!has_file && !has_file_base)
    return 2;

  // 3.) Provided a file base but not a suffix
  if (has_file_base && !has_file_suffix)
    return 3;

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
  std::vector<std::string> filenames;

  if (has_file)
    filenames.push_back(file);

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
    for (int i = 0; i < dir.n_files; i++)
    {
      // Update the current file
      tinydir_file file;
      tinydir_readfile_n(&dir, &file, i);

      // Store the file if it has proper extension as in numeric range
      if (!file.is_dir && MooseUtils::hasExtension(file.name, file_suffix))
      {
        std::string the_base;
        unsigned int file_num = 0;
        file_base_and_num_regex.FullMatch(file.name, &the_base, &file_num);

        if (!the_base.empty() && file_num >= file_range[0] && file_num <= file_range[1])
          filenames.push_back(split_file.first + "/" + file.name);
      }
    }
    tinydir_close(&dir);
  }

  else
    mooseError("We'll never get here!");

  // for (unsigned i=0; i<filenames.size(); ++i)
  //   Moose::out << "In parseFileRange() filenames[" << i << "]=" << filenames[i] << std::endl;

  params.set<std::vector<std::string> >("filenames") = filenames;

  // If we made it here, there were no errors
  return 0;
}



std::string getFileRangeErrorMessage(int code)
{
  switch (code)
  {
  case 0:
    return std::string("");
  case 1:
    return std::string("Cannot provide both file and file_base parameters");
  case 2:
    return std::string("You must provide a valid value for either the 'file' parameter or the 'file_base' parameter.");
  case 3:
    return std::string("If you provide a 'file_base', you must also provide a valid 'file_suffix', e.g. 'png'.");
  default:
    return std::string("Unkown error code!");
  }
}
