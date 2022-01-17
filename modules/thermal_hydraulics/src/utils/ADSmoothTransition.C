#include "ADSmoothTransition.h"

ADSmoothTransition::ADSmoothTransition(const ADReal & x_center, const ADReal & transition_width)
  : _x_center(x_center),
    _transition_width(transition_width),
    _x1(_x_center - 0.5 * _transition_width),
    _x2(_x_center + 0.5 * _transition_width)
{
}
