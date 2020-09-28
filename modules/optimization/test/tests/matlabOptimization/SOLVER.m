classdef SOLVER < handle
    properties
        Stiffness
        Force
        Displacement
        % MOOSE: executable as an atribute
        % as well as the total ndof
    end
    
    methods
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        % MOOSE: Change the argument to the name of the
        % executable and just set the name of the file
        function obj=SOLVER(K)
            obj.Stiffness = K;
            obj.Force = 0;
            obj.Displacement = 0;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        % MOOSE: Set the executable
        function obj=SetStiffness(obj,K)
            obj.Stiffness = K;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function obj=SetForce(obj,F)
            % MOOSE: Update the Moose input file and obtain force
            obj.Force = F;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function obj = Solve(obj)
            % MOOSE: Call the executable and set the 
            % displacement
            
            %  FIXME LYNN % set current estimate of parameters and then solve..... will call moose with rhs= obj.Force
            % need to write the parameters .  convert parameters to force vector and solve.
            % multiply the parameters by the expansion matrix.
            
            %this really just needs to write Force to file, call solve and then read in
            %the results into Displacement
            R1=1; %skip column names
            C1=0;
            
            coordFilename = "zForwardInput/initialCoords.csv";
            data = csvread(coordFilename,R1,C1); % doing this to get coords
            data(:,5)=obj.Force;
            inputFilename = "zForwardInput/inputForces.csv";
            csvwrite(inputFilename,data);
            
            system('../../../isopod-opt -i ./forwardSolve.i > junk.txt');
            
            outputFilename="zForwardOutput/all_temperatures_0002.csv";
            data = csvread(outputFilename,R1,C1);
            obj.Displacement = data(:,5);
            
            %obj.Displacement = obj.Stiffness\obj.Force;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function sol = ForwardSolve(obj,rhs)
            % MOOSE: write the rhs to a file and 
            % run the excutable and set the displacement

            R1=1; %skip column names
            C1=0;
            
            coordFilename = "zForwardInput/initialCoords.csv";
            data = csvread(coordFilename,R1,C1); % doing this to get coords
            data(:,5)=rhs;
            inputFilename = "zForwardInput/inputForces.csv";
            csvwrite(inputFilename,data);
            
            system('../../../isopod-opt -i ./forwardSolve.i > junk.txt');

            outputFilename="zForwardOutput/all_temperatures_0002.csv";
            data = csvread(outputFilename,R1,C1);
            sol = data(:,5);
            %sol = obj.Stiffness\rhs;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function sol = AdjointSolve(obj,rhs)
            % MOOSE: Do the same as ForwardSolve
            R1=1; %skip column names
            C1=0;
            
            coordFilename = "zAdjointInput/initialCoords.csv";
            data = csvread(coordFilename,R1,C1); % doing this to get coords
            data(:,5)=rhs;
            inputFilename = "zAdjointInput/inputForces.csv";
            csvwrite(inputFilename,data);

            system('../../../isopod-opt -i ./adjointSolve.i > junk.txt');

            outputFilename="zAdjointOutput/all_temperatures_0002.csv";
            data = csvread(outputFilename,R1,C1);
            sol = data(:,5);
            %sol = (obj.Stiffness')\rhs;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function nodalDof = NodalDofFromState(~,state)
            % MOOSE keep as is for now
            nodalDof = state;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function state = StateFromNodalDof(~,nodalDof)
            % MOOSE keep as is for now
            state = nodalDof;
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        function nstates = NumStates(obj)
            % MOOOSE output total ndof
            nstates = size(obj.Stiffness,1);
        end
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    end
end
