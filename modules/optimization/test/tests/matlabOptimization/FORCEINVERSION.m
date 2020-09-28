classdef FORCEINVERSION < handle
    properties
        Parameter
        Solver
        ObjectiveFunction
    end
    
    methods
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function obj=FORCEINVERSION(parameter,solver,objectiveFunction)
            obj.Parameter = parameter;
            obj.Solver = solver;
            obj.ObjectiveFunction = objectiveFunction;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function SetParameterForSolver(obj,parameterV)
            nodalForceV = obj.Parameter.NodalForceFromParameter(parameterV);
            obj.Solver.SetForce(nodalForceV);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        % This requires the application of the extension
        % operator, i.e. E matrix
        function stateV = ApplyCp(obj,parameterV)
            nodalForceV = obj.Parameter.NodalForceFromParameter(parameterV);
            stateV = obj.Solver.StateFromNodalDof(nodalForceV);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        % This requires application of transpose(E)
        function parameterV = ApplyCpAdjoint(obj,stateV)
            nodalForceV = obj.Solver.NodalDofFromState(stateV);
            parameterV = obj.Parameter.ParameterFromNodalForce(nodalForceV);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function val = ObjectiveFunctionValue(obj,parameterV)
            obj.SetParameterForSolver(parameterV);
            obj.Solver.Solve();
            stateV = obj.Solver.Displacement();
            val = obj.ObjectiveFunction.Value(stateV,parameterV);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function gradientV = Gradient(obj,parameterV)
            obj.SetParameterForSolver(parameterV);
            obj.Solver.Solve();
            stateV = obj.Solver.Displacement();
            JuV = obj.ObjectiveFunction.Ju(stateV);
            w = obj.Solver.AdjointSolve(-JuV);
            gradientV = obj.ObjectiveFunction.Jp(parameterV) + obj.ApplyCpAdjoint(w);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function resultV = ApplyHessian(obj,stepV)
            V1 = obj.ApplyCp(stepV);
            V2 = obj.Solver.ForwardSolve(V1);
            V3 = obj.ObjectiveFunction.ApplyLuu(V2) - obj.ObjectiveFunction.ApplyLup(stepV);
            V4 = obj.Solver.AdjointSolve(V3);
            resultV = obj.ApplyCpAdjoint(V4) - obj.ObjectiveFunction.ApplyLpu(V2) + obj.ObjectiveFunction.ApplyLpp(stepV);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    end
end