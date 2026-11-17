# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html

"""Re-evaluate a NEML2 constitutive update on the exact batch that failed inside
a MOOSE run, using the per-rank TorchScript file written by
``dump_inputs_on_failure``, and read the failure context straight off the raised
exception. The two functions below are the copy-paste boilerplate listed in
NEML2Action.md; the unittest guards them against regression (the TestHarness runs
it against the dump produced by its prereq test)."""

import os

# Ask NEML2 to attach a failure context to the recoverable ConvergenceError a
# non-converged constitutive update raises. Must be set before the failing solve
# (both the compiled and Python runtimes read it live). Off by default because it
# costs an extra masked re-solve on failure.
os.environ["NEML2_CAPTURE_SOLVE_FAILURE"] = "1"

import glob
import unittest

import torch

import neml2
from neml2.solvers import ConvergenceError


def load_dumped_inputs(dump_glob, model_input, model_name):
    """Load a ``dump_inputs_on_failure`` TorchScript file and map its buffers back
    onto a freshly-loaded NEML2 model -- no MOOSE required.

    dump_glob   : glob for the per-rank dump(s) MOOSE named in its error message,
                  e.g. ``"mymodel_count0_rank*.pt"``
    model_input : the model ``.i`` (eager) or the ``<model>_aoti.i`` stub (compiled)
    model_name  : the ``[Models]`` block name
    Returns ``(model, inputs)`` where ``inputs`` maps each model input name to its
    NEML2-typed tensor from the dump.
    """

    # dump_inputs_on_failure stores each input tensor as a TorchScript buffer,
    # replacing characters that are invalid in identifiers -- the ``~`` of
    # old-state lag names, the ``/`` of nested names -- with ``_``. Re-apply the
    # same sanitization to match a model input name to its buffer.
    def sanitize(name):
        return "".join(c if (c.isalnum() or c == "_") else "_" for c in name)

    dump_file = sorted(glob.glob(dump_glob))[0]  # one file per failing rank
    buffers = dict(torch.jit.load(dump_file).named_buffers())

    model = neml2.load_model(model_input, model_name)
    inputs = {name: T(buffers[sanitize(name)]) for name, T in model.input_spec.items()}
    return model, inputs


def diagnose_failure(model, inputs):
    """Re-run the model on the whole failing batch and read the failure context.

    With ``NEML2_CAPTURE_SOLVE_FAILURE`` set (see top of file), the recoverable
    ``ConvergenceError`` carries, as attributes:
      ``converged_mask`` -- a per-batch-entry bool tensor (``True`` where that
                            quadrature point converged),
      ``unknowns``       -- ``{unknown-variable name -> best-effort iterate}``,
                            the state the Newton solve got stuck at. Each value is
                            a NEML2-typed tensor when replaying an eager model or a
                            plain ``torch.Tensor`` for a compiled one; both carry
                            the dynamic-batch leading dim, so index them the same.
    Returns ``(failing_indices, unknowns)`` -- the batch entries (quadrature
    points) that diverged and the stuck state -- or ``None`` if the batch
    actually converged on replay (the MOOSE-context failure did not reproduce
    standalone). Export ``NEML2_LOGS="newton=info"`` to also print the
    per-iteration residual history of the replay.
    """
    try:
        model(*[inputs[name] for name in model.input_spec])
        return None  # converged on replay -- nothing to diagnose
    except ConvergenceError as e:
        failing = (~e.converged_mask).nonzero().flatten().tolist()
        return failing, e.unknowns


class DumpedInputReplayTest(unittest.TestCase):
    def test_replay_and_diagnose(self):
        model, inputs = load_dumped_inputs(
            "model_count*_rank*.pt", "models/failing_model.i", "model"
        )
        # The boilerplate is wired up: every model input was found in the dump.
        self.assertEqual(set(inputs), set(model.input_spec))

        result = diagnose_failure(model, inputs)
        # The dump is, by construction, a failing batch, so the replay reproduces
        # the recoverable failure and returns a populated context.
        self.assertIsNotNone(result)
        failing, unknowns = result
        # At least one quadrature point diverged, and every index is in range.
        self.assertTrue(len(failing) > 0)
        nbatch = next(x.batch_shape[0] for x in inputs.values() if x.batch_shape)
        self.assertTrue(all(0 <= i < nbatch for i in failing))
        # The stuck state is reported per solved-for unknown, each a tensor-like
        # value (NEML2-typed here, since this replays an eager model) carrying the
        # dynamic-batch leading dim.
        self.assertTrue(len(unknowns) > 0)
        self.assertTrue(all(v.shape[0] == nbatch for v in unknowns.values()))


if __name__ == "__main__":
    unittest.main()
