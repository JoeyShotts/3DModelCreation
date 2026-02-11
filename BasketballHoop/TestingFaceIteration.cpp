
#include <thread>
#include <fstream> 
#include <iostream>
#include <cmath>
#include <vector>
#include <string.h>
#include <chrono>

#define numDivVert (int) 5 //must be even
#define numDivHoriz (int) 5 //must be even

int main(){
    //random seed
    srand(static_cast<unsigned int>(time(0)));
    
    int numFaces = numDivHoriz*numDivVert; 
    int faceToRandIndex[numFaces];
    int faceID; //integer divide by numDivHoriz to get row(0-numDivVert), modulo numDivHoriz to get column(0-numDivHoriz)

    //get a list of all values needed
    for(int i=0; i<numFaces;i++){
        faceToRandIndex[i] = i;
    }

    int row, col, randIndex, saveVal;

    //shuffle list of all faces
    for(int i=0; i<numFaces;i++){
        randIndex = rand()%numFaces;
        saveVal = faceToRandIndex[randIndex];
        faceToRandIndex[randIndex] = faceToRandIndex[i];
        faceToRandIndex[i] = saveVal;
    }

    for(int i=0; i<numFaces; i++){
        faceID = faceToRandIndex[i];
        
        //get current face
        row= faceID/numDivHoriz;
        col= faceID % numDivHoriz;
        std::cout<<"FaceID: "<<faceID<<" R: "<<row<<" C:"<<col<<"\n";
    }
    return 0;
}