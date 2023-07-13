#include <iostream>
#include <chrono>
#include <thread>
#include <mpi.h>
#include <cstring>
void some_here(int rank)
{
  int secret_data = 42;

  // Share the secret data with all ranks
  MPI_Bcast(&secret_data, 1, MPI_INT, 0, MPI_COMM_WORLD);

  //std::cout << "          [some_here] Rank: " << rank << " Secret Data: " << secret_data << "\n" << std::endl;
}

void some_there(int &stack_depth)
{

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  //std::cout << "          [some_there] Hi, I'm rank: " << rank << " off doing something else and waiting\n" << std::endl;
  if(rank == stack_depth)//This is what causes the program to hang. One rank will broadcast, none of the others will respond.
  {
    //std::cout<<"\n          [some_there]My rank is " << rank << " and I am calling some_there."<< std::endl;
    some_here(rank);//broadcasts
  }

  if ((rank < stack_depth) || (rank > stack_depth))//These ranks just wait, and never respond to the poor divergent rank.
  {
   // std::cout<<"\n          [some_there]My rank is " << rank << " and I am off doing something else.\n"<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(65));
  }
}

int parse_s_arguement(int &argc, char** &argv);

void specific_stack_hang(int &stack_depth, int counter);

int main(int argc, char **argv) {

  MPI_Init(&argc, &argv);

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  //std::cout << "[main] World Size: " << world_size <<std::endl;

  int divergent_stack_num = 0;
  divergent_stack_num = parse_s_arguement(argc, argv);

  std::this_thread::sleep_for(std::chrono::seconds(5));//resource-cheap way to get the ranks roughly in order with output.
  specific_stack_hang(divergent_stack_num,0);

  MPI_Finalize();
  return 0;
}

//Parses input from command line
int parse_s_arguement(int &argc, char** &argv)
{
  int divergent_stack_num;
  int i;
  int temp = i;

  for (i=0; i<argc; i++)
  {
    //printf("argv[%d]: %s\n", i, argv[i]);

    //will be true when they are the same. Looking for -s flag to specifiy number of stacks deep program will hang
    if (strncmp(argv[i], "-s",2)==0){

      divergent_stack_num = atoi(argv[i+1]);//TODO once this works, add input validation to check that it is only positive integers.
      //printf("     [parse_s_arguement] You entered %d number of stacks.\n", divergent_stack_num);
      return static_cast<int>(divergent_stack_num);
    }
  }
  //printf("     -s not specified, number of stacks set to default.\n");
  return 4;
}

void specific_stack_hang(int &stack_depth, int counter)
{
  int count_up = counter;

  while(count_up < (stack_depth-1))//This controls the "depth" of the hang.
  {
    //std::cout<<"     [specific_stack_hang] count_up is "<< count_up << std::endl;
    count_up++;
    specific_stack_hang(stack_depth, count_up);
  }

  if(count_up == (stack_depth-1))//all ranks should call this function so that they diverge outside of main
  {
    //std::cout<<"         [specific_stack_hang] count_up:" << count_up << " == stack_depth, exiting specific_stack_hang" << std::endl;
    some_there(stack_depth);
  }
}
