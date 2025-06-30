from abc import ABC, abstractmethod
from typing import Dict, Union
import json


class CouplingData:
    """stores the coupling varaible and defines the mapping both ways"""

    def __init__(self, json_file) -> None:
        with open(json_file) as file:
            d = json.load(file)
        # maps from MOOSE to FMU vice versa
        self.MOOSE_to_FMU: Dict[str, str] = {k: v for k, v in d["mapping"]}
        self.FMU_to_MOOSE: Dict[str, str] = {v: k for k, v in d["mapping"]}

        # FMU inputs with type
        self.from_MOOSE: Dict[str, str] = d["from_MOOSE"]
        # FMU ouputs with type
        self.to_MOOSE: Dict[str, str] = d["to_MOOSE"]


class FMUBase(ABC):

    @abstractmethod
    def __init__(self, endTime, json_file: str = "FMU.json"):
        self.coupleData = CouplingData(json_file)
        self.endTime = endTime

    @abstractmethod
    def setVar(self, key: str, val: float) -> None:
        pass

    @abstractmethod
    def getVar(self, key: str) -> float:
        pass

    @abstractmethod
    def stepUntil(self, t):
        pass

    @abstractmethod
    def __del__(self):
        pass

    def setValue(self, setV, var) -> None:
        of_key, val = var
        key = self.coupleData.OF_to_FMU[of_key]
        var_type = self.coupleData.from_OF[of_key]
        if var_type == "REAL":
            setV(key, val)

    def getValue(self, getV, of_key) -> float:
        key = self.coupleData.OF_to_FMU[of_key]
        var_type = self.coupleData.to_OF[of_key]
        if var_type == "REAL":
            return getV(key)

    def from_MOOSE(self, dumped_input_json: str):
        ext_inputs = json.loads(dumped_input_json)
        for k in self.coupleData.from_OF:
            var = (k, ext_inputs[k])
            self.setValue(self.setVar, var)

    def to_MOOSE(self) -> str:
        d = {}
        for k in self.coupleData.to_OF:
            d[k] = self.getValue(self.getVar, k)
        return str(json.dumps(d))
