% TODO

function HessianV = HessVec(parameterV,grad)
global GlobalK;
global GlobalUm;
global GlobalProj;
global GlobalExpan;
global GlobalTikhonov;
solver = SOLVER(GlobalK);
data = DATA(GlobalProj,GlobalUm);
parameter = FORCEPARAMETER(GlobalExpan);
objectiveFunction = OBJECTIVEFUNCTION(parameter,solver,data,GlobalTikhonov);
forceInversion = FORCEINVERSION(parameter,solver,objectiveFunction);
value = forceInversion.ObjectiveFunctionValue(parameterV);
gradient = forceInversion.Gradient(parameterV);
% the following is inefficient
nparams = size(parameterV,1);
IdentityMatrix = eye(nparams);
Hessian = zeros(nparams);
for i=1:nparams
    Hessian(:,i) = forceInversion.ApplyHessian(IdentityMatrix(:,i));
end
HessianV = -forceInversion.ApplyHessian(parameterV);
end