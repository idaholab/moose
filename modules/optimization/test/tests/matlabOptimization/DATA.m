classdef DATA < handle
    properties
        ProjectionMatrix
        DataVector
    end
    
    methods
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function obj = DATA(projectionMatrix,dataVector)
            obj.ProjectionMatrix = projectionMatrix;
            obj.DataVector = dataVector;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function obj=SetProjectionMatrix(obj,P)
            obj.ProjectionMatrix = P;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function nodalDofV = NodalDofFromData(obj,dataV)
            nodalDofV = obj.ProjectionMatrix'*dataV;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function dataV = DataFromNodalDof(obj,nodalDofV)
            dataV = (obj.ProjectionMatrix)*nodalDofV;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function dataV = Value(obj)
            dataV = obj.DataVector;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    end
end