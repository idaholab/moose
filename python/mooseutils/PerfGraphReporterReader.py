# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements the PerfGraphReporterReader for reading from the PerfGraphReporter."""

from moosepy.perfgraph import PerfGraph

from mooseutils.ReporterReader import ReporterReader


class PerfGraphReporterReader(PerfGraph):
    """A Reader for MOOSE PerfGraphReporterReader data."""

    def __init__(self, file: str, part: int = 0):
        """
        Initialize.

        Parameters
        ----------
        file : str
            Path to the reporter output.

        Optional Parameters
        -------------------
        part : int
            The reporter part; defaults to 0.

        """
        reader = ReporterReader(file)
        reader.update(part=part)

        # Find the Reporter variable that contains the PerfGraph graph
        perf_graph_var = None
        for var in reader.variables():
            if reader.info(var[0])["type"] == "PerfGraphReporter" and var[1] == "graph":
                perf_graph_var = var

        if perf_graph_var is None:
            raise ValueError("Failed to find PerfGraphReporter in output")

        data = reader[perf_graph_var]
        assert isinstance(data, dict)
        PerfGraph.__init__(self, data)
