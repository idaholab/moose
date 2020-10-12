function [value,gradient,Hessian] = ForceInversionFGH(parameterV)
global GlobalK;
global GlobalUm;
global GlobalProj;
global GlobalExpan;
global GlobalTikhonov;

% MOOSE - replace the following with Moose executable
solver = SOLVER(GlobalK);

data = DATA(GlobalProj,GlobalUm);
parameter = FORCEPARAMETER(GlobalExpan);
objectiveFunction = OBJECTIVEFUNCTION(parameter,solver,data,GlobalTikhonov);
forceInversion = FORCEINVERSION(parameter,solver,objectiveFunction);


% Building explicit Hessian from HessVec product function
% The following is should be be replaced by directly passing the
% HessVec product, especially for material inversion
nparams = size(parameterV,1);
IdentityMatrix = eye(nparams);
Hessian = zeros(nparams);
for i=1:nparams
    Hessian(:,i) = forceInversion.ApplyHessian(IdentityMatrix(:,i));
end

gradient = forceInversion.Gradient(parameterV);
value = forceInversion.ObjectiveFunctionValue(parameterV);



end