//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimedSubdomainModifier.h"
#include "TimedElementSubdomainModifier.h"
#include "DelimitedFileReader.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", TimedSubdomainModifier);

InputParameters
TimedSubdomainModifier::validParams()
{
  InputParameters params = TimedElementSubdomainModifier::validParams();

  // parameters for direct input
  params.addParam<std::vector<Real>>("times", "The times of the subdomain modifications.");
  params.addParam<std::vector<SubdomainName>>("blocks_from",
                                              "Names or ids of the 'old' block(s), to be renamed.");
  params.addParam<std::vector<SubdomainName>>("blocks_to", "Names or ids of the 'new' block.");
  params.addParamNamesToGroup("times blocks_from blocks_to", "Subdomain change data from input");

  // parameters for file-based data supply
  params.addParam<FileName>("data_file", "File holding CSV data");
  params.addParam<bool>("header",
                        "Indicates whether the file contains a header with the column names");
  params.addParam<std::string>("delimiter", ",", "Delimiter used to parse the file");
  params.addParam<std::string>("comment", ";", "Comment character used to parse the file");
  params.addParam<std::size_t>(
      "time_column_index", 0, "Zero-based index of the time column. Default is '0'.");
  params.addParam<std::size_t>(
      "blocks_from_column_index", 1, "Zero-based index of the blocks_from column. Default is '1'.");
  params.addParam<std::size_t>(
      "blocks_to_column_index", 2, "Zero-based index of the blocks_to column. Default is '2'.");
  params.addParam<std::string>("time_column_text", "Header text of the time column.");
  params.addParam<std::string>("blocks_from_column_text", "Header text of the blocks_from column.");
  params.addParam<std::string>("blocks_to_column_text", "Header text of the blocks_to column.");
  params.addParamNamesToGroup(
      "data_file header delimiter comment time_column_index blocks_from_column_index "
      "blocks_to_column_index time_column_text blocks_from_column_text blocks_to_column_text",
      "Subdomain change data from CSV file");

  params.addClassDescription(
      "Modify element subdomain ID of entire subdomains for given points "
      "in time. This mesh modifier only runs on the undisplaced mesh, and it will "
      "modify both the undisplaced and the displaced mesh.");

  return params;
}

TimedSubdomainModifier::TimedSubdomainModifier(const InputParameters & parameters)
  : TimedElementSubdomainModifier(parameters)
{
  // determine function arguments

  // helper variables for parameter consistency checks
  const auto from_data_file =
      isParamSetByUser("data_file") + isParamSetByUser("header") + isParamSetByUser("delimiter") +
      isParamSetByUser("comment") + isParamSetByUser("time_column_index") +
      isParamSetByUser("blocks_from_column_index") + isParamSetByUser("blocks_to_column_index") +
      isParamSetByUser("time_column_text") + isParamSetByUser("blocks_from_column_text") +
      isParamSetByUser("blocks_to_column_text");

  const auto from_data_File_needs_header = isParamSetByUser("time_column_text") +
                                           isParamSetByUser("blocks_from_column_text") +
                                           isParamSetByUser("blocks_to_column_text");

  const auto from_parameters =
      isParamSetByUser("times") + isParamSetByUser("blocks_from") + isParamSetByUser("blocks_to");

  // Check parameter set for inconsistencies
  const auto from_source_count = ((from_data_file > 0) + (from_parameters > 0));
  if (from_source_count != 1)
  {
    mooseError("Data on times and blocks must be provided either via a CSV file ('data_file' and "
               "corresponding blocks), or via direct parameter input ('times', 'blocks_from', and "
               "'blocks_to').");
  }

  if (from_parameters > 0)
  {
    if (from_parameters != 3)
      mooseError("All parameters 'times', and 'blocks_from', and 'blocks_to' must be specified.");
    buildFromParameters();
  }
  else if (from_data_file > 0)
  {
    if ((isParamSetByUser("time_column_index") + isParamSetByUser("time_column_text")) > 1)
      mooseError(
          "The parameters 'time_column_index', and 'time_column_text' are mutual exclusive.");
    if ((isParamSetByUser("blocks_from_column_index") +
         isParamSetByUser("blocks_from_column_text")) > 1)
      mooseError("The parameters 'blocks_from_column_index', and 'blocks_from_column_text' are "
                 "mutual exclusive.");
    if ((isParamSetByUser("blocks_to_column_index") + isParamSetByUser("blocks_to_column_text")) >
        1)
      mooseError("The parameters 'blocks_to_column_index', and 'blocks_to_column_text' are mutual "
                 "exclusive.");
    if ((from_data_File_needs_header > 0) && (isParamSetByUser("header") == 0))
      mooseError("Header flag must be active if columns are to be found via header.");

    buildFromFile();
  }
  else
    mooseError("Unknown data source. Are you missing a parameter? Did you misspell one?");
}

void
TimedSubdomainModifier::buildFromParameters()
{
  _times = getParam<std::vector<Real>>("times");
  const auto n = _times.size();

  const auto raw_from = getParam<std::vector<SubdomainName>>("blocks_from");
  const auto raw_to = getParam<std::vector<SubdomainName>>("blocks_to");

  if (raw_from.size() != n)
    mooseError(
        "Parameter 'blocks_from' must contain the same number of items as parameter 'times'.");
  if (raw_to.size() != n)
    mooseError("Parameter 'blocks_to' must contain the same number of items as parameter 'times'.");

  _blocks_from.resize(n);
  _blocks_to.resize(n);

  for (const auto i : index_range(raw_from))
  {
    _blocks_from[i] = getSubdomainIDAndCheck(raw_from[i]);
    _blocks_to[i] = getSubdomainIDAndCheck(raw_to[i]);
  }
}

void
TimedSubdomainModifier::buildFromFile()
{
  const auto _file_name = getParam<FileName>("data_file");

  /// Flag indicating if the file contains a header.
  auto _header_flag = MooseUtils::DelimitedFileOfStringReader::HeaderFlag::OFF;
  if (isParamValid("header"))
  {
    _header_flag = getParam<bool>("header")
                       ? MooseUtils::DelimitedFileOfStringReader::HeaderFlag::ON
                       : MooseUtils::DelimitedFileOfStringReader::HeaderFlag::OFF;
  }

  std::string _delimiter = ",";
  if (isParamValid("delimiter"))
  {
    _delimiter = getParam<std::string>("delimiter");
  }

  std::string _comment = "#";
  if (isParamValid("comment"))
  {
    _comment = getParam<std::string>("comment");
  }

  MooseUtils::DelimitedFileOfStringReader file(_file_name);

  file.setHeaderFlag(_header_flag);
  file.setDelimiter(_delimiter);
  file.setComment(_comment);
  file.read();

  std::size_t _time_column = 0;
  if (isParamValid("time_column_text"))
  {
    const auto s = getParam<std::string>("time_column_text");
    const auto _names = file.getNames();
    const auto it = find(_names.begin(), _names.end(), s);
    if (it == _names.end())
      mooseError("Could not find '", s, "' in header of file ", _file_name, ".");
    _time_column = std::distance(_names.begin(), it);
  }
  else if (isParamValid("time_column_index"))
  {
    _time_column = getParam<std::size_t>("time_column_index");
  }

  std::size_t _blocks_from_column = 1;
  if (isParamValid("blocks_from_column_text"))
  {
    const auto s = getParam<std::string>("blocks_from_column_text");
    const auto _names = file.getNames();
    const auto it = find(_names.begin(), _names.end(), s);
    if (it == _names.end())
      mooseError("Could not find '", s, "' in header of file ", _file_name, ".");
    _blocks_from_column = std::distance(_names.begin(), it);
  }
  else if (isParamValid("blocks_from_column_index"))
  {
    _blocks_from_column = getParam<std::size_t>("blocks_from_column_index");
  }

  std::size_t _blocks_to_column = 2;
  if (isParamValid("blocks_to_column_text"))
  {
    const auto s = getParam<std::string>("blocks_to_column_text");
    const auto _names = file.getNames();
    const auto it = find(_names.begin(), _names.end(), s);
    if (it == _names.end())
      mooseError("Could not find '", s, "' in header of file ", _file_name, ".");
    _blocks_to_column = std::distance(_names.begin(), it);
  }
  else if (isParamValid("blocks_to_column_index"))
    _blocks_to_column = getParam<std::size_t>("blocks_to_column_index");

  const auto max_needed_column_index =
      std::max({_time_column, _blocks_from_column, _blocks_to_column});

  const auto data = file.getData();

  if (data.size() < max_needed_column_index)
    mooseError("data must contain at least " + std::to_string(max_needed_column_index) +
                   " columns in file ",
               _file_name);

  const auto strTimes = data[_time_column];
  const auto strBlockFrom = data[_blocks_from_column];
  const auto strBlockTo = data[_blocks_to_column];

  const auto n_rows = strTimes.size();

  // some sanity checks
  if (n_rows == 0)
    mooseError("empty sequence in file ", _file_name);
  if (n_rows != strBlockFrom.size())
    mooseError("Inconsistent source block data size in ",
               _file_name,
               ". Expected ",
               n_rows,
               " and read ",
               strBlockFrom.size());
  if (n_rows != strBlockTo.size())
    mooseError("Inconsistent target block data size in ",
               _file_name,
               ". Expected ",
               n_rows,
               " and read ",
               strBlockTo.size());

  // resize variables to fit the data
  _blocks_from.resize(n_rows);
  _blocks_to.resize(n_rows);

  // fill the to and from blocks vectors
  for (const auto & time_str : strTimes)
    _times.push_back(std::stod(time_str));
  std::transform(strBlockFrom.begin(),
                 strBlockFrom.end(),
                 _blocks_from.begin(),
                 [this](const std::string & x) { return getSubdomainIDAndCheck(x); });
  std::transform(strBlockTo.begin(),
                 strBlockTo.end(),
                 _blocks_to.begin(),
                 [this](const std::string & x) { return getSubdomainIDAndCheck(x); });
}

SubdomainID
TimedSubdomainModifier::getSubdomainIDAndCheck(const std::string & subdomain_name)
{
  const auto id = _mesh.getSubdomainID(subdomain_name);
  if (id == Moose::INVALID_BLOCK_ID)
    mooseError("block", "Subdomain \"" + subdomain_name + "\" not found in mesh.");
  return id;
}

SubdomainID
TimedSubdomainModifier::computeSubdomainID()
{
  // get the subdomain-id of the current element
  SubdomainID resulting_subdomain_id = _current_elem->subdomain_id();

  // check for all the subdomain changes that can have been requested between the previous and the
  // current time
  for (const auto & time_pair : _times_and_indices)
  {
    // time of the data point
    const auto t = time_pair.time;

    // original data point index
    const auto j = time_pair.index;

    // do we have to apply?
    if (t > _t_old && t <= _t && resulting_subdomain_id == _blocks_from[j])
    {
      // we have to change the subdomain-id using the original index (stored in 'j')
      resulting_subdomain_id = _blocks_to[j];
    }
  }

  return resulting_subdomain_id;
}
