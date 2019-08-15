/*
This pipeline builds a Docker image of MOOSE, runs the standard suite of tests,
and pushes the image to the registry prescribed by $DOCKER_TAG.  A second push 
is also done, with the tag suffix set to the commit hash of the MOOSE working 
tree.  For this all to work, $DOCKER_TAG must be prescribed as a pipeline 
parameter in Jenkins.
*/

pipeline {
    agent none
    
    stages {
        
        // Run 'docker build' on the Dockerfile in this repository
        // with args for core count and submodule commit hashes
        stage('Build Image')
        {
            agent any
            
            steps {
                sh '''
                # Setup for getting submodule commit hashes
                git submodule init
                sub_rev() { git submodule status $1 | awk '{print $1}' ; }
                
                # Build MOOSE image with max cores and compatible submodules
                docker build \
                --build-arg MOOSE_JOBS=$(grep -c ^processor /proc/cpuinfo) \
                --build-arg PETSC_REV=$(sub_rev petsc) \
                --build-arg LIBMESH_REV=$(sub_rev libmesh) \
                -t $DOCKER_TAG .
                '''
            }
        }
        
        // Pipes output from MOOSE tests into a readable, accessible file
        stage('Run Tests')
        {
            agent any
            
            steps {
                sh '''
                # Run tests and pipe to accessible log file
                docker run $DOCKER_TAG /bin/bash -c \
                'cd test; ./run_tests' | tee test_results.log
                
                # Remove characters from text coloring
                sed -ie 's/\\x1b\\[[0-9;]*m//g' test_results.log
                '''
            }
        }
        
        // Pushes given tag to registry and also for the current MOOSE commit
        stage('Push to Docker Registry')
        {
            agent any
            
            steps {
                sh '''
                # Get Docker repo prefix and tag string for current revision
                DOCKER_REPO=$(echo $DOCKER_TAG | awk -F ':' '{print $1}')
                DOCKER_COMMIT_TAG="$DOCKER_REPO:$(git rev-parse HEAD)"
                
                # Push given tag and also tag by commit
                docker push $DOCKER_TAG
                docker tag $DOCKER_TAG $DOCKER_COMMIT_TAG
                docker push $DOCKER_COMMIT_TAG
                '''
            }
        }
    }
}