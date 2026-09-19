# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""MOOSE-hosted NEML2 model: rate of a rank-two tensor from its increment.

This is a NEML2 model authored and shipped by MOOSE (solid_mechanics). It is
imported into the embedded cpp-eager interpreter via a NEML2Action ``load``
parameter, which resolves this file on the application's data search path. Python
port of the v2 C++ ``R2IncrementToRate`` -- the crystal-plasticity bridge that
turns a MOOSE-provided incremental deformation gradient into the rate form NEML2
consumes.
"""

from __future__ import annotations

from neml2.factory import register_neml2_object
from neml2.models.chain_rule import ChainRuleDict
from neml2.models.model import Model
from neml2.schema import HitSchema, derived_input, input, output
from neml2.types import R2, Scalar


@register_neml2_object("R2IncrementToRate")
class R2IncrementToRate(Model):
    r"""``rate = increment / (t - t~1)``.

    Compute the rate of a rank-two tensor from its increment over the time step.
    The forward is linear in ``increment`` and a simple reciprocal in time, so the
    chain rule mirrors ``common/VariableRate`` (with the increment standing in for
    the ``v - v~1`` difference).
    """

    hit = HitSchema(
        input("increment", R2, "Increment of the rank two tensor", attr="_incr"),
        input("time", Scalar, "Current time", default="t", attr="_t"),
        derived_input("time", Scalar, attr="_tn", suffix="~1"),
        output("rate", R2, "Rate of the rank two tensor", attr="_rate"),
    )

    # Auto-declared by ``from_hit`` -- resolved variable names, no __init__ needed.
    _incr: str
    _t: str
    _tn: str
    _rate: str

    def forward(  # type: ignore[override]
        self,
        increment: R2,
        t: Scalar,
        t_n: Scalar,
        v: ChainRuleDict | None = None,
    ):
        dt = t - t_n
        rate = increment / dt
        if v is None:
            return rate

        # Differential pushforwards (matching the v2 set_value derivatives):
        #   d rate / d increment = I / dt        -> V / dt
        #   d rate / d t         = -rate / dt    -> -(rate / dt) * V  (V scalar)
        #   d rate / d t~1       =  rate / dt    ->  (rate / dt) * V  (V scalar)
        rate_over_dt = rate / dt
        actions = {
            self._incr: lambda V, c=dt: V / c,
            self._t: lambda V, c=rate_over_dt: -(c * V),
            self._tn: lambda V, c=rate_over_dt: c * V,
        }
        return rate, self.apply_chain_rule(v, self._rate, actions, output=rate)
