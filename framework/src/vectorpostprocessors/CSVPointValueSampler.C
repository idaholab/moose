#include "CSVPointValueSampler.h"
#include "DelimitedFileReader.h"

registerMooseObject("MooseApp", CSVPointValueSampler);

InputParameters
CSVPointValueSampler::validParams()
{
  InputParameters params = PointVariableSamplerBase::validParams();
  params.addClassDescription("Sample a variable at points specified in a csv file.");
  params.addRequiredParam<FileName>("points_file", "CSV file with point coordinates (x, y, z).");
  params.addRequiredParam<std::string>("point_xcoord",
                                       "x coordinate column name from csv file being read in.");
  params.addRequiredParam<std::string>("point_ycoord",
                                       "y coordinate column name from csv file being read in.");
  params.addRequiredParam<std::string>("point_zcoord",
                                       "z coordinate column name from csv file being read in.");
  params.addParam<std::string>("point_id", "id column name from csv file being read in.");

  return params;
}

CSVPointValueSampler::CSVPointValueSampler(const InputParameters & parameters)
  : PointVariableSamplerBase(parameters)
{
  std::string idName;
  if (isParamValid("point_id"))
    idName = getParam<std::string>("point_id");

  std::string xName = getParam<std::string>("point_xcoord");
  std::string yName = getParam<std::string>("point_ycoord");
  std::string zName = getParam<std::string>("point_zcoord");

  bool found_x = false;
  bool found_y = false;
  bool found_z = false;

  std::vector<Real> point_xcoord;
  std::vector<Real> point_ycoord;
  std::vector<Real> point_zcoord;

  MooseUtils::DelimitedFileReader reader(getParam<FileName>("points_file"));
  reader.read();

  auto const & names = reader.getNames();
  auto const & data = reader.getData();

  for (std::size_t i = 0; i < names.size(); ++i)
  {
    if (names[i] == xName)
    {
      point_xcoord = data[i];
      found_x = true;
    }
    else if (names[i] == yName)
    {
      point_ycoord = data[i];
      found_y = true;
    }
    else if (names[i] == zName)
    {
      point_zcoord = data[i];
      found_z = true;
    }
    else if (names[i] == idName)
    {
      _ids = data[i];
    }
  }
  if (!found_x || !found_y || !found_z)
    paramError("points_file",
               "Column with name '",
               xName,
               " or ",
               yName,
               " or ",
               zName,
               "' missing from measurement file");

  const std::size_t nvals = point_xcoord.size();
  _points.resize(nvals);
  for (const auto & i : make_range(nvals))
  {
    const Point point(point_xcoord[i], point_ycoord[i], point_zcoord[i]);
    _points[i] = point;
  }
}
