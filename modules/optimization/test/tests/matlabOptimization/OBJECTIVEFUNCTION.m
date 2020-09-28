classdef OBJECTIVEFUNCTION < handle
    properties
        Data;
        Parameter; % needed for regularization
        TikhonovParameter;
        Solver;    % needed for reduced order modeling
    end
    
    methods
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function obj = OBJECTIVEFUNCTION(parameter,solver,data,alpha)
            obj.Parameter = parameter;
            obj.Solver = solver;
            obj.Data = data;
            obj.TikhonovParameter = alpha;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function obj=SetTikhonovPatameter(obj,alpha)
            obj.TikhonovPatameter = alpha;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function val = Value(obj,stateV,parameterV)
            misfit = obj.Data.Value() - obj.DataFromState(stateV);
            val = 0.5*(misfit')*misfit + 0.5*obj.TikhonovParameter*(parameterV')*parameterV;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function stateV2 = ApplyLuu(obj,stateV1)
            dataV = obj.DataFromState(stateV1);
            stateV2 = obj.StateFromData(dataV);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function stateV = ApplyLup(obj,parameterV)
            stateV = zeros(obj.Solver.NumStates,1);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function parameterV = ApplyLpu(obj,stateV)
            parameterV = zeros(obj.Parameter.NumParameters,1);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function parameterV2 = ApplyLpp(obj,parameterV1)
            parameterV2 = obj.TikhonovParameter*parameterV1;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function dataV = DataFromState(obj,stateV)
            nodalDofV = obj.Solver.NodalDofFromState(stateV);
            dataV = obj.Data.DataFromNodalDof(nodalDofV);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function stateV = StateFromData(obj,dataV)
            nodalDofV = obj.Data.NodalDofFromData(dataV);
            stateV = obj.Solver.StateFromNodalDof(nodalDofV);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function JpV = Jp(obj,parameterV)
            JpV = obj.TikhonovParameter*parameterV;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function JuV = Ju(obj,stateV)
            misfit = obj.Data.Value() - obj.DataFromState(stateV);
            JuV = obj.StateFromData(misfit);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    end
end