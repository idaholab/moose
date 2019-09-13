#include "PsbtPowerIC.h"

registerMooseObject("MooseApp", PsbtPowerIC);

template <>
InputParameters
validParams<PsbtPowerIC>()
{
  InputParameters params = validParams<PsbtIC>();
  params.addParam<Real>("total_power", 413.0, "Combined power produced by all pins in [kW]");
  return params;
}

PsbtPowerIC::PsbtPowerIC(const InputParameters & params)
  : PsbtIC(params)
{
  auto total_power = getParam<Real>("total_power");

  // Define a reference power.  This is the amount of power one full-power pin
  // applies to one channel (assuming one quarter of the pin's power goes to
  // each of its four neighboring channels).
  constexpr int n_full_power {12};
  constexpr int n_quarter_power {13};
  Real ref_power = 0.25 * total_power
                   / (1.0*n_full_power + 0.25*n_quarter_power);

  // Convert the reference power to a linear heat rate.
  constexpr Real heated_length {3.658};  // in m
  _ref_qprime = ref_power / heated_length;  // in kW/m
}

Real
PsbtPowerIC::value(const Point & p)
{
  // Determine which channel this point is in.
  auto inds = index_point(p);
  auto i = inds.first;
  auto j = inds.second;

  // Figure out how much heat is applied to this location.  We have to account
  // for boundary channels which might only border 1 or 2 pins.  We also have
  // to account for the different pin powers.  The power distribution is
  // f f q q q   where f is a full power pin and q is a quarter-power pin.
  // f f f q q
  // f f q q q
  // f f f q q
  // f f q q q
  if (i < 2) {
    // This channel borders full-power pins.  Figure out how many pins it
    // touches and return the power.
    auto n_pins = 4;
    if (i == 0) n_pins /= 2;
    if (j == 0 || j == 5) n_pins /= 2;
    return _ref_qprime * n_pins;

  } else if (i > 3) {
    // This channel borders quarter-power pins.  Figure out how many pins it
    // touches and return the power.
    auto n_pins = 4;
    if (i == 5) n_pins /= 2;
    if (j == 0 || j == 5) n_pins /= 2;
    return (0.25*_ref_qprime) * n_pins;

  } else if (i == 2) {
    // This channel touches a mix of full-power and quarter-power pins.
    if (j == 0 || j == 5) {
      return _ref_qprime + (0.25*_ref_qprime);
    } else {
      return 3 * _ref_qprime + (0.25*_ref_qprime);
    }

  } else { // i == 3
    // This channel touches a mix of full-power and quarter-power pins.
    if (j == 0 || j == 5) {
      return 2 * (0.25*_ref_qprime);
    } else {
      return _ref_qprime + 3 * (0.25*_ref_qprime);
    }
  }
}
