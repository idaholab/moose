#include "SideAverageValueRZ.h"

template<>
InputParameters validParams<SideAverageValueRZ>()
{
  InputParameters params = validParams<SideIntegralRZ>();
  return params;
}

SideAverageValueRZ::SideAverageValueRZ(const std::string & name, InputParameters parameters) :
    SideIntegralRZ(name, parameters),
    _area(0),
    _current_element_area(0)
{}

void
SideAverageValueRZ::initialize()
{
  SideIntegral::initialize();
  _area = 0;
}

void
SideAverageValueRZ::execute()
{
  SideIntegral::execute();
  _area += _current_element_area;
}

Real
SideAverageValueRZ::getValue()
{
  Real integral = SideIntegralRZ::getValue();

  gatherSum(_area);

  return integral / _area;
}


void
SideAverageValueRZ::threadJoin(const Postprocessor & y)
{
  SideIntegral::threadJoin(y);
  const SideAverageValueRZ & pps = dynamic_cast<const SideAverageValueRZ &>(y);
  _area += pps._area;
}

Real
SideAverageValueRZ::computeIntegral()
{
  Real sum(0);
  _current_element_area = 0;

  for (_qp=0; _qp< _qrule->n_points(); _qp++)
  {
    sum += _JxW[_qp]*computeQpIntegral();
    _current_element_area += 2 * M_PI * _q_point[_qp](0) * _JxW[_qp];
  }
  return sum;
}
