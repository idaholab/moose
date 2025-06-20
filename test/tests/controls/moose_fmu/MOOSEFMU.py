from pythonfmu import Fmi2Causality, Fmi2Slave, Boolean, Integer, Real, String
import numpy as np
from MooseControl import MooseControl
import os
import uuid


class MooseFMU(Fmi2Slave):

    description = "Get postprocessor value from MOOSE"

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        # Initialized MOOSEControl
        self.moose = MooseControl(moose_port=12345)
        self.moose.initialize()
        self.moose.wait("INITIAL")

        self.intOut = 1
        self.realOut = 3.0
        self.booleanVariable = True
        self.stringVariable = "Hello MOOSE!"
        self.register_variable(Integer("intOut", causality=Fmi2Causality.output))
        self.register_variable(Real("realOut", causality=Fmi2Causality.output))
        self.register_variable(
            Boolean("booleanVariable", causality=Fmi2Causality.local)
        )
        self.register_variable(String("stringVariable", causality=Fmi2Causality.local))

    def doStep(self, currentCommunicationPoint, communicationStepSize):

        self.moose.setContinue()

        moose_time_size = self.moose.getPostprocessor("dt")
        num_time_step = round(communicationStepSize / moose_time_size)

        self.moose.wait("TIMESTEP_BEGIN")
        for i in range(num_time_step):
            self.realOut = self.moose.getPostprocessor("t")
            self.moose.setContinue()

        print(
            f"Step completed: Time = {currentCommunicationPoint + communicationStepSize}"
        )
        return True

    def getReal(self, refs):
        return self.realOut

    def terminate(self):
        if self.moose:
            print("Terminating MOOSE simulation...")
            self.moose.finalize()
            self.moose = None
        print("MOOSE FMU terminated!")
