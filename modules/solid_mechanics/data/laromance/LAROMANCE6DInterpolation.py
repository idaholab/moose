# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""MOOSE-hosted NEML2 model: LAROMANCE 6D multilinear interpolation grid.

This is a NEML2 model authored and shipped by MOOSE (solid_mechanics). It is
imported into the embedded cpp-eager interpreter via a NEML2Action ``load``
parameter, which resolves this file on the application's data search path. Python
port of the v2 C++ ``LAROMANCE6DInterpolation`` -- a reduced-order creep model
that samples the inelastic strain rate and the cell/wall dislocation density
rates from a six-dimensional rectilinear grid (read from a JSON file) by
multilinear interpolation, with per-axis input transforms and an output
transform.

The grid axes, in order, are
``[von_mises_stress, temperature, equivalent_plastic_strain,
cell_dislocation_density, wall_dislocation_density, env_factor]``; the schema
declares the six model inputs plus one output rate selected by
``model_file_variable_name`` (``out_ep`` / ``out_cell`` / ``out_wall``).

For ``out_ep`` the model declares ``request_AD`` for ``d(output_rate) /
d(von_mises_stress)`` only -- mirroring the v2 model, which requested that single
first derivative for the implicit radial-return solve. The leaf therefore writes
only a value ``forward`` (no ``v=`` branch); the framework supplies the chain
rule by reverse-mode autodiff. request_AD is fully supported in cpp-eager (which
is what these tests use); the cell/wall instances are forward-only.

The grids are registered as buffers so they track the model's working device and
dtype under ``.to()`` (NEML2 builds typed tensors at float64) and so the eager
surface infers the model dtype from them.
"""

from __future__ import annotations

import itertools
import json
from typing import cast

import torch

from neml2.factory import register_neml2_object
from neml2.models.model import Model
from neml2.schema import HitSchema, input, option, output
from neml2.types import Scalar

# Grid axes, in interpolation order. The forward inputs are reordered onto this
# axis order before interpolation; the 6D grid values are indexed in this order.
_GRID_KEYS = ("in_stress", "in_temperature", "in_plastic_strain", "in_cell", "in_wall", "in_env")


@register_neml2_object("LAROMANCE6DInterpolation")
class LAROMANCE6DInterpolation(Model):
    """Multilinear interpolation over six dimensions.

    Inputs (von_mises_stress, temperature, equivalent_plastic_strain,
    cell_dislocation_density, wall_dislocation_density, env_factor) and a single
    output rate selected from the JSON grid by ``model_file_variable_name``.
    """

    hit = HitSchema(
        input("equivalent_plastic_strain", Scalar, "The equivalent plastic strain"),
        input("von_mises_stress", Scalar, "The von Mises stress", attr="_vm"),
        input("cell_dislocation_density", Scalar, "The cell dislocation density"),
        input("wall_dislocation_density", Scalar, "The wall dislocation density"),
        input("temperature", Scalar, "The temperature"),
        input("env_factor", Scalar, "The environment factor"),
        output("output_rate", Scalar, "The output rate", attr="_out_rate"),
        option("model_file_name", str, "The name of the model file", attr="_model_file"),
        option(
            "model_file_variable_name",
            str,
            "The name of the variable in the model file",
            attr="_out_name",
        ),
    )

    # Auto-declared by the schema (_store_schema_values). ``_vm`` is the resolved
    # von Mises stress variable name; the options hold the JSON file and the
    # selected output variable name.
    _vm: str
    _out_rate: str
    _model_file: str
    _out_name: str

    def __post_init__(self) -> None:
        with open(self._model_file) as f:
            j = json.load(f)

        # 1D grids (buffers) and per-axis input transforms, in grid-axis order.
        for i, key in enumerate(_GRID_KEYS):
            self.register_buffer(
                f"_grid_{i}", torch.tensor(self._require(j, key), dtype=torch.float64)
            )
        self._in_transforms = [self._read_transform(j, key) for key in _GRID_KEYS]

        # 6D grid values for the selected output (validate rectangularity first,
        # matching the v2 json_6Dvector_to_torch size check).
        values = self._require(j, self._out_name)
        self._check_rectangular(values, self._out_name)
        self.register_buffer("_grid_values", torch.tensor(values, dtype=torch.float64))

        # Output transform, keyed off the selected output variable.
        if self._out_name == "out_ep":
            self._out_transform = self._read_transform(j, "out_strain_rate")
        elif self._out_name == "out_cell":
            self._out_transform = self._read_transform(j, "out_cell_rate")
        elif self._out_name == "out_wall":
            self._out_transform = self._read_transform(j, "out_wall_rate")
        else:
            raise RuntimeError(
                "This ouput variable is not implemented, model_file_variable_name: "
                + str(self._out_name)
            )

        # Only the inelastic strain rate needs a first derivative, w.r.t. the von
        # Mises stress, for the radial-return solve (matches the v2 model).
        if self._out_name == "out_ep":
            self.request_AD(inputs=[self._vm])  # type: ignore[operator]

    def forward(  # type: ignore[override]
        self,
        equivalent_plastic_strain: Scalar,
        von_mises_stress: Scalar,
        cell_dislocation_density: Scalar,
        wall_dislocation_density: Scalar,
        temperature: Scalar,
        env_factor: Scalar,
        *nl_params: Scalar,
        v=None,
    ):
        # Transform each input onto its grid axis (raw-tensor boundary, matching
        # the ML-surrogate request_AD pattern: the grad-tracking leaf request_AD
        # swaps in for von_mises_stress is exactly ``.data``), then interpolate
        # and transform the result back.
        points = [
            self._transform(von_mises_stress.data, self._in_transforms[0]),
            self._transform(temperature.data, self._in_transforms[1]),
            self._transform(equivalent_plastic_strain.data, self._in_transforms[2]),
            self._transform(cell_dislocation_density.data, self._in_transforms[3]),
            self._transform(wall_dislocation_density.data, self._in_transforms[4]),
            self._transform(env_factor.data, self._in_transforms[5]),
        ]
        out = Scalar(self._transform(self._interpolate(points), self._out_transform))
        if v is None:
            return out

        # out_ep's first derivative (w.r.t. von_mises_stress) is supplied by
        # request_AD, so __call__ intercepts before forward ever sees ``v``. The
        # cell/wall instances reach here with a ``v`` seed; like the v2 model they
        # carry no input derivative, so the chain rule is a structural zero.
        return out, self.apply_chain_rule(v, self._out_rate, {}, output=out)

    # ------------------------------------------------------------------
    # Interpolation
    # ------------------------------------------------------------------

    def _interpolate(self, points: list[torch.Tensor]) -> torch.Tensor:
        """64-corner (2^6) multilinear interpolation over the rectilinear grid."""
        # Fetch the grid buffers fresh so they reflect the current device/dtype
        # after any .to() move.
        grids = [getattr(self, f"_grid_{a}") for a in range(6)]
        grid_values = cast(torch.Tensor, self._grid_values)

        left_idx: list[torch.Tensor] = []
        # ``fracs[axis] = (left_weight, right_weight)`` for the bracketing pair.
        fracs: list[tuple[torch.Tensor, torch.Tensor]] = []
        for grid, pt in zip(grids, points):
            n = grid.shape[0]
            # searchsorted returns the right index; -1 makes it the left. Clamp
            # to [0, n-2] so the bracketing pair stays in range (extrapolation
            # falls out of the fraction formula below).
            idx = torch.clamp(torch.searchsorted(grid, pt) - 1, 0, n - 2)
            left = grid[idx]
            right = grid[idx + 1]
            left_fraction = (right - pt) / (right - left)
            left_idx.append(idx)
            fracs.append((left_fraction, 1.0 - left_fraction))

        result = torch.zeros_like(points[0])
        for corner in itertools.product((0, 1), repeat=6):
            vertex = grid_values[tuple(left_idx[a] + corner[a] for a in range(6))]
            weight = fracs[0][corner[0]]
            for a in range(1, 6):
                weight = weight * fracs[a][corner[a]]
            result = result + vertex * weight
        return result

    # ------------------------------------------------------------------
    # Transforms (data <-> grid coordinate). ``params`` is (type, values).
    # ------------------------------------------------------------------

    @staticmethod
    def _transform(data: torch.Tensor, params: tuple[str, list[float]]) -> torch.Tensor:
        ttype, p = params
        if ttype == "COMPRESS":
            factor, compressor, original_min = p[0], p[1], p[2]
            d1 = torch.sign(data) * torch.abs(data * factor) ** compressor
            return torch.log10(1.0 + d1 - original_min)
        if ttype == "DECOMPRESS":
            factor, compressor, original_min = p[0], p[1], p[2]
            d1 = 10.0**data - 1.0 + original_min
            return torch.sign(d1) * torch.abs(d1) ** (1.0 / compressor) / factor
        if ttype == "LOG10BOUNDED":
            factor, lowerbound, upperbound, logmin, logmax = p[0], p[1], p[2], p[3], p[4]
            rng = upperbound - lowerbound
            return rng * (torch.log10(data + factor) - logmin) / (logmax - logmin) + lowerbound
        if ttype == "EXP10BOUNDED":
            factor, lowerbound, upperbound, logmin, logmax = p[0], p[1], p[2], p[3], p[4]
            rng = upperbound - lowerbound
            return 10.0 ** (((data - lowerbound) * (logmax - logmin) / rng) + logmin) - factor
        if ttype == "MINMAX":
            data_min, data_max, scaled_min, scaled_max = p[0], p[1], p[2], p[3]
            return (data - data_min) / (data_max - data_min) * (scaled_max - scaled_min) + scaled_min
        raise RuntimeError("Unrecognized transform: " + str(ttype))

    # ------------------------------------------------------------------
    # JSON readers
    # ------------------------------------------------------------------

    @staticmethod
    def _require(j: dict, key: str):
        if key not in j:
            raise RuntimeError(f"The key '{key}' is missing from the JSON data file.")
        return j[key]

    def _read_transform(self, j: dict, axis: str) -> tuple[str, list[float]]:
        return (
            self._require(j, axis + "_transform_type"),
            self._require(j, axis + "_transform_values"),
        )

    @staticmethod
    def _check_rectangular(data, key: str) -> None:
        """Validate the 6D grid is rectangular (every sub-list at a given depth
        matches the size of the first sub-list at that depth), matching the v2
        ``json_6Dvector_to_torch`` size check."""

        def walk(v):
            if not isinstance(v, list) or not v or not isinstance(v[0], list):
                return
            n = len(v[0])
            for x in v:
                if len(x) != n:
                    raise RuntimeError(f"Incorrect JSON interpolation grid size for '{key}'.")
                walk(x)

        walk(data)


__all__ = ["LAROMANCE6DInterpolation"]
