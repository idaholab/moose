# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""MOOSE-hosted NEML2 model(s) for the framework test suite.

This module demonstrates hosting a custom NEML2 model *inside* a MOOSE app under
the v3 (Python-native) model surface. Importing the module fires the
``@register_neml2_object`` decorator, registering the type in NEML2's native
registry so it resolves from a HIT input file. MOOSE brings the module into the
embedded (cpp-eager) interpreter via the ``NEML2Action`` ``load`` parameter,
which forwards it to ``neml2.cli._extensions.load_user_extensions`` -- the same
mechanism ``neml2-compile`` uses for the cpp-aoti route.

``NEML2TestModel`` is the Python port of the v2 C++ ``NEML2TestModel`` test model.
"""

from __future__ import annotations

from neml2.factory import register_neml2_object
from neml2.models.chain_rule import ChainRuleDict
from neml2.models.model import Model
from neml2.schema import HitSchema, input, option, output, parameter
from neml2.types import Scalar


@register_neml2_object("NEML2TestModel")
class NEML2TestModel(Model):
    r"""``sum = p1 * A + p2 * B`` and ``product = p1 * A * p2 * B``.

    Two scalar inputs (``A``, ``B``), two scalar parameters (``p1``, ``p2``,
    default 1), two scalar outputs. The forward is linear in each input, so the
    input chain rule is a constant scaling per input; parameter derivatives are
    obtained from reverse-mode AD via ``param_jacobian`` and need no hand-coded
    action here.

    ``ad`` is accepted for compatibility with the v2 model variants but is a
    no-op: the analytic chain rule below is always used (NEML2 v3 forbids
    ``torch.autograd`` inside a native ``forward`` since it does not survive
    export). ``error`` makes the forward raise, to exercise MOOSE's handling of a
    failed NEML2 evaluation (cpp-eager only).
    """

    hit = HitSchema(
        input("A", Scalar, "Input variable A"),
        input("B", Scalar, "Input variable B"),
        output("sum", Scalar, "Output variable sum"),
        output("product", Scalar, "Output variable product"),
        parameter("p1", Scalar, "Parameter p1", default="1", allow_promotion=True),
        parameter("p2", Scalar, "Parameter p2", default="1", allow_promotion=True),
        option("ad", bool, "Accepted for v2 compatibility; the analytic chain rule is always used",
               default=True, attr="ad"),
        option("error", bool, "Raise during evaluation, to test error handling", default=False,
               attr="error"),
    )

    # Auto-declared by ``from_hit`` -- no __init__ needed.
    p1: Scalar
    p2: Scalar
    ad: bool
    error: bool

    def forward(  # type: ignore[override]
        self,
        A: Scalar,
        B: Scalar,
        *nl_params: Scalar,
        v: ChainRuleDict | None = None,
    ):
        if self.error:
            raise RuntimeError("NEML2TestModel error flag is set")

        p1 = self._get_param("p1", nl_params, Scalar)
        p2 = self._get_param("p2", nl_params, Scalar)

        sum_out = p1 * A + p2 * B
        product_out = p1 * A * p2 * B

        if v is None:
            return sum_out, product_out

        v_sum = self.apply_chain_rule(
            v,
            "sum",
            {"A": lambda V: p1 * V, "B": lambda V: p2 * V},
            output=sum_out,
        )
        v_product = self.apply_chain_rule(
            v,
            "product",
            {"A": lambda V: (p1 * p2 * B) * V, "B": lambda V: (p1 * p2 * A) * V},
            output=product_out,
        )
        return sum_out, product_out, {**v_sum, **v_product}
