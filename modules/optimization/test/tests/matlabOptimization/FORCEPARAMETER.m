classdef FORCEPARAMETER < handle
    properties
        ExpansionMatrix
    end
    
    methods
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function obj=FORCEPARAMETER(expansionMatrix)
            obj.ExpansionMatrix = expansionMatrix;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function obj=SetExpansionMatrix(obj,E)
            obj.ExpansionMatrix = E;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function nodalForceV = NodalForceFromParameter(obj,parameterV)
            nodalForceV = obj.ExpansionMatrix*parameterV;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function parameterV = ParameterFromNodalForce(obj,nodalForceV)
            parameterV = obj.ExpansionMatrix'*nodalForceV;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function nParameters = NumParameters(obj)
            nParameters = size(obj.ExpansionMatrix,2);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    end
end