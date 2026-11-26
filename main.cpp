// Distributed Systems - 2026
// Assignment 2 - template
// Student Name: Mohamed Fdael Bucharaya El Manssouri
// student ID: 3089448

#include <iostream>
#include <fstream>
#include <mpi.h>
#include <string>
#include <limits>
#include <cstring>

//Global variables
const int arraySize = 550;
int world_size, world_rank;
int key;
char totalArray[arraySize];

//createData helper method - Do not modify
void createData(int rank) {
    /* DATA METHOD
    This method populates an array with the text read from file.
    */

    // File must be saved in the same directory as the executable of this program
    std::string nodeFileName = "ciphertext.txt"; //read your corresponding .txt file
    std::ifstream MyReadFile(nodeFileName);
    // Only 1 node need read the file, output to console
    std::cout << "Node " << rank << " reading file: " << nodeFileName << std::endl;
    //check if file is read correctly
    if (!MyReadFile.is_open()) {
        std::cerr << "Unable to open file: " << nodeFileName << std::endl;
        std::cerr << "Make sure ciphertext.txt is in the same directory as your executable." << std::endl;
        return;
    }
    else{
        MyReadFile.getline(totalArray, arraySize);
        std::cout << "File read successfully." << std::endl;
    }
    // Close the file
    MyReadFile.close();
}
//decryptText helper method - Do not modify
char* decryptText(int key, char* arr, int size) {
    int length, i;
    char c;
    //loop through each string in array
    for (int j = 0; j < size; j++) {
        c = arr[j];
        //if character is in alphabet, convert to uppercase
        if (isalpha(c)) {
            c = toupper(c);
            //decrypt using formula (c - key % 26)
            c = 'A' + (((c - 'A') - key + 26) % 26);
            arr[j] = c;
        }
    }
    return arr;
}
//searchText helper method - Do not modify
int searchText(char* arr, int size, const char* needle) {
    int hits = 0;
    char* found = arr;
    //search given array for the needle
    while ((found = strstr(found, needle)) != NULL && found - arr < size) {
        hits++;
        found++;
    }
    return hits;
}
//exportText helper method - Do not modify
void exportText(char* arr, int size, const char* filename) {
    std::ofstream outfile(filename); //create new file
    outfile.write(arr, size); //write array to file
    outfile.close(); //close the file
}

//student details print function
void printStudentDetails() {
    std::cout << "Student Name: Mohamed Fadel Bucharaya El Manssouri" << std::endl;
    std::cout << "Student ID: 3089448" << std::endl;
}

//print array fonction
void printArray(char* arr, int size){
    for (int i=0; i<size; i++){
        std::cout << arr[i];
    }
}

int main(int argc, char** argv) {
    // TODO: Work to be done here

    //initialzing mpi environment
    MPI_Init(NULL, NULL);

    ////how many processes in the MPI instance
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    //which rank we are in
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    //print student details only once by node 0
    if (world_rank == 0) {
        printStudentDetails();
    }

    //question a part *************************************************************

    //node 0 asking user for key input + key velidation
    if (world_rank == 0) {
        std::cout <<'\n' << "Please enter the key for decryption (1-25): ";
        //making sure key is an INT
        while (true) {
            if (!(std::cin >> key)) {
                std::cout << "Invalid input. The key has to be a integer number. " << '\n' << "Try again: ";
                std::cin.clear();//clearing key input
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            //after key passes the int check, we check if it is in the range of 1-25
            if (key >= 1 && key <= 25) {
                break;
            }
            else {
                std::cout << "Invalid input. The key has to be between 1 and 25. " << '\n' << "Try again: ";
            }
        } //NOTE: for some reason if I input for example 7UIK or 8U etc its still stores the first number (8 or 7 in the example) as the key, ignoring the rest of the letters.
    
        //printing key once to console by node 0
        std::cout << '\n' <<"your decryption key is: " << key << '\n' <<std::endl;
    }

    //synchronizing bofore node 0 broadcasting
    MPI_Barrier( MPI_COMM_WORLD);


    //node 0 will broadcast the key to all other nodes
    MPI_Bcast(&key, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //debugging- checking that all process received the key
    /*std::cout << "Node " << world_rank << " received key: " << key << std::endl;*/

    // question b part *************************************************************

    //partitioning the array equally bettwen nodes (declaring)
    int partitionArraySize = arraySize / world_size;
    char partitionArray[partitionArraySize];

    if (world_rank == 0) {
        createData(world_rank); //node 0 will read the cypher text file and populate the array
    }
    //using mpi scatter to give equal partition array to all nodes off the total array
    MPI_Scatter( totalArray , partitionArraySize , MPI_CHAR , partitionArray , partitionArraySize , MPI_CHAR , 0 , MPI_COMM_WORLD);

    //debugging: making sure every node recieved the partition array by printinh the first carachter if the array
    /*MPI_Barrier( MPI_COMM_WORLD);//synchronizing before printing
    std::cout << '\n' <<"Node " << world_rank << " recived its partition array and its first charachter is: " << partitionArray[0]<<std::endl;
    */


    // question c part *************************************************************

    //each node will decypher its chunk 
    decryptText(key, partitionArray, partitionArraySize);
    //storing into the new array
    char decipheredArray[partitionArraySize];
    for(int i = 0 ; i < partitionArraySize ; i++){
        decipheredArray[i] = partitionArray[i];
    }
    //each node print its deciphered array
    std::cout << '\n' << "Node " << world_rank << " has decipherd the array and its chunk is: " <<std::endl;
    printArray(decipheredArray,partitionArraySize);
    std::cout << std::endl;


    // question d part *************************************************************

    //synchronizing node for better consol output
    MPI_Barrier( MPI_COMM_WORLD);
    //each node calls search text method on its deciphered array
    const char* needle = "DISTRIBUTED";
    int hits = searchText(decipheredArray, partitionArraySize, needle);
    //each node returned the amount of hits it found
    std::cout << '\n' <<"Node: " << world_rank << " found " << hits << " hits." <<std::endl;


    // question e part *************************************************************

    //node 0 will gather all the hits from all nodes
    MPI_Barrier( MPI_COMM_WORLD);
    int totalHits = 0;
    MPI_Reduce(&hits, &totalHits, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    //node 0 will broadcast the total hits to all other nodes so node 1 can be able to print it.
    MPI_Bcast(&totalHits, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //node 1 will print the total hits found
    if (world_rank == 1) {
        std::cout << '\n' <<"Distributed was found a total of " << totalHits << " times." <<std::endl;
    }


    // question f part *************************************************************

    //strlen of each deciphered array.
    int count = strlen(decipheredArray);
    //gather all counts from all nodes to node 0
    int newTotalArraySize;
    MPI_Gather(&count, 1, MPI_INT, &newTotalArraySize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //using mpi gather, node 0 will collect all the deciphered arrays from other nodes and put them together.
    MPI_Barrier( MPI_COMM_WORLD);//synchronizing before gathering
    char* decriptedTotalArray = new char[newTotalArraySize];
    MPI_Gather(decipheredArray,newTotalArraySize, MPI_CHAR,decriptedTotalArray,newTotalArraySize,MPI_CHAR,0,MPI_COMM_WORLD);
    //expoting the total deciphered array to a new file
    const char* filename = "MohamedFadelBucharayaDecryptedText.txt";
    exportText(decriptedTotalArray, newTotalArraySize, filename);



    //finalize mpi environment
    MPI_Finalize();
    
    //return 0 to OS
    return 0;
   

}