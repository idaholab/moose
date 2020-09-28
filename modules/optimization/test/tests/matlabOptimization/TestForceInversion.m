clear all;
global GlobalK;
global GlobalUm;
global GlobalProj;
global GlobalExpan;
global GlobalTikhonov;

parameter_node_ids = [69 73 157 161]; %node ids of forcing
data_node_ids = [36 40 190 194]; %node ids of measurements
data_node_values = [100 200 300 400]'; %node ids measurement value

dataSize = length(data_node_ids);            % number of measurement points
parameterSize = length(parameter_node_ids);  % number of forcing parameters
stateSize = 231;                             % ndof of the forward problem

GlobalK = rand(1);             % Random (unsymmetric) stiffness matrix //fixme lynn don't need
GlobalUm = data_node_values;   % Random values as measured data
GlobalProj = zeros(dataSize,stateSize);        % Projection matrix is also random
GlobalExpan = zeros(stateSize,parameterSize);  % Force parameter expansion matrix is also random
GlobalTikhonov = 0;                            % No Tikhonov regularization

for i=1:size(GlobalProj,1)
    for j=1:size(GlobalProj,2)
        if j == data_node_ids(i)
            GlobalProj(i,j)=1;
        end
    end
end
for i=1:size(GlobalExpan,1)
    for j=1:size(GlobalExpan,2)
        if i == parameter_node_ids(j)
            GlobalExpan(i,j)=1;
        end
    end
end


% MOOSE: Construct GlobalProj, GlobalExpan from CSV file
%        No need for GlobalK - keep it as dummy variable
%        Read GlobalUm from CSV file

Finit = zeros(parameterSize,1); % Initial estimate of force = 0

options = optimoptions(@fminunc,...
                       'Display','iter',...
                       'Algorithm','trust-region',... % need to figure out a way to make the radius large
                       'SpecifyObjectiveGradient',true,...
                       'HessianFcn','objective',...
                       'MaxPCGIter',100,...
                       'OptimalityTolerance',1e-10,...
                       'StepTolerance',1e-10,...
                       'FunctionTolerance',1e-10...
                       );
% The following will be done after figuring out how to write HessVec
% function
%                       'HessianFcn',[],...
%                       'HessianMultiplyFcn',@HessVec,...
[invertedForce,finalObjective,exitflag,output] = fminunc(@ForceInversionFGH,Finit,options);

% Checking the accuracy of inversion by solving for the displacments at the
% observation points
expandedForce = GlobalExpan*invertedForce;
StateFromForce = GlobalK\expandedForce;
UmFromForce = GlobalProj*StateFromForce;
errorForce = norm(UmFromForce-GlobalUm)/norm(GlobalUm)
