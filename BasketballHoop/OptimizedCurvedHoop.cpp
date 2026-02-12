/*
Joseph Shotts
10/8/2025
C++ Code
Description: 
Creating a simple curved backboard.
*/

//Problems:
// The output seems only to be on one side, and seems to be random. 
// The problem cause could be the trajectory function, the faceTest function, the small test size,
// the ML algorithm doesn't work, or some other unknown problem.

// Also could update time estimate to be some kind of running average to get more acurate estimation.
// Also time estimate still isn't acurate, I think because when threads need to be run multiple times, thread average isn't accurate.
// Could try to move some of the functions across a couple files.
// Also there should be some kind of clean up function for the STL class.
// Could try to utilize a gpu for faster calculations.

//Fix:
// Investigate the FaceTest and look for something obvious (why is it only on half?)
// Try testing at a larger sample size.
// Write some tests for the trajectory function.
// Then do some more research into ML algroithims.

#include <thread>
#include <fstream> 
#include <iostream>
#include <cmath>
#include <vector>
#include <string.h>
#include <chrono>
#include <crtdbg.h>

//for STL Class
#define FLOAT_E (float)1e-09 //used for float comparison

struct TriFloatXYZ{
    float X;
    float Y;
    float Z;
};

struct Triangle{
    TriFloatXYZ *normal;
    TriFloatXYZ *P1;
    TriFloatXYZ *P2;
    TriFloatXYZ *P3;
};

//used when testing individual faces in seperate threads
struct faceTest{
    TriFloatXYZ P1;
    TriFloatXYZ P2;
    TriFloatXYZ P3;
    TriFloatXYZ P4;
    float performance;
    int64_t time;
    bool  testCompleted;
};

//critical values used to define shape backboard, essentially defines number of faces
#define numDivVert (int)10 //must be even
#define numDivHoriz (int)20 //must be even

//functions used to test face
const int numOptimizations = 0;
void optimizeBackboardCurve(Triangle* curvedFrontTris[numDivVert][numDivHoriz*2], float avgWidthHeight);
void testFace(void* faceIn);
void findTrajectory(TriFloatXYZ* ballEnd, TriFloatXYZ* bouncePoint, TriFloatXYZ* ballStart, TriFloatXYZ* faceNV, float maxH);

//STL Class Definitions **************************************************************
//for new point
// TriFloatXYZ *nP(float X, float Y, float Z){
//     TriFloatXYZ *point = new TriFloatXYZ;
//     point->X = X;
//     point->Y = Y;
//     point->Z = Z;
//     return point;
// }

void copyPoint(TriFloatXYZ *copiedPoint, TriFloatXYZ *origPoint){
    copiedPoint->X=origPoint->X;
    copiedPoint->Y=origPoint->Y;
    copiedPoint->Z=origPoint->Z;
}

//calculates unit normal of face and sets parameter normal to that value
//this function is self contained as it's intended to use externally of the STL Class
void getNormal(Triangle* tri){
    TriFloatXYZ VectorP1P2; // = subtractTwoPoints(triangle->P1, triangle->P2);
    VectorP1P2.X = tri->P1->X - tri->P2->X;
    VectorP1P2.Y = tri->P1->Y - tri->P2->Y;
    VectorP1P2.Z = tri->P1->Z - tri->P2->Z;

    TriFloatXYZ VectorP1P3; // = subtractTwoPoints(triangle->P1, triangle->P3);
    VectorP1P3.X = tri->P1->X - tri->P3->X;
    VectorP1P3.Y = tri->P1->Y - tri->P3->Y;
    VectorP1P3.Z = tri->P1->Z - tri->P3->Z;

    TriFloatXYZ *V1 = &VectorP1P2;
    TriFloatXYZ *V2 = &VectorP1P3;

    //calculate unitvector
    tri->normal->X = (V1->Y * V2->Z) - (V1->Z*V2->Y); //i
    tri->normal->Y = (V1->Z * V2->X) - (V1->X*V2->Z); //j
    tri->normal->Z = (V1->X * V2->Y) - (V1->Y*V2->X); //k

    //find magnitude and divide to get unit vector
    float magnitude = tri->normal->X*tri->normal->X + tri->normal->Y*tri->normal->Y;
    magnitude += tri->normal->Z*tri->normal->Z;
    magnitude = sqrt(magnitude);
    tri->normal->X = tri->normal->X /magnitude;
    tri->normal->Y = tri->normal->Y /magnitude;
    tri->normal->Z = tri->normal->Z /magnitude;
}

class STL_Binary{
public:
    Triangle* addTriangle(TriFloatXYZ *P1, TriFloatXYZ *P2, TriFloatXYZ *P3){
        Triangle *newFace = new Triangle;
        newFace->P1 = findPoint(P1);
        newFace->P2 = findPoint(P2);
        newFace->P3 = findPoint(P3);

        normalUnitVector(newFace);
        faces.push_back(newFace);
        return newFace;
    }

    //searches all points (by float comparison), returns point if found, 
    //else returns new point
    TriFloatXYZ* findPoint(TriFloatXYZ* P){
        TriFloatXYZ compare;
        //loop through all points
        for(auto v:vertices){
            compare.X = ::fabs(P->X - v->X); //get x difference
            if(compare.X > FLOAT_E){ //compare x difference against tiny epsilon value
                continue; //if different enough go to next vertice
            }
            compare.Y = ::fabs(P->Y - v->Y);
            if(compare.Y > FLOAT_E){
                continue;
            }
            compare.Z = ::fabs(P->Z - v->Z);
            if(compare.Z > FLOAT_E){
                continue;
            }
            //return the point if all coordinates are close enough
            return v;
        }
        //create new point, add it to vertices vector, return it
        TriFloatXYZ* newPoint = new TriFloatXYZ;
        copyPoint(newPoint, P);
        vertices.push_back(newPoint);
        return newPoint;
    }

    int numTriangles(){
        return faces.size();
    }

    void renderSTL(const std::string& name){
        std::ofstream stl_stream(name, std::ios::out | std::ios::binary);

        if (!stl_stream.is_open()) {
            std::cerr << "Error: Unable to open file for writing." << std::endl;
            return; // Indicate an error
        }

        printBuffer(stl_stream);
        printNumTri(stl_stream);
        for (Triangle* face : faces) {
            normalUnitVector(face); //recalculate the unit vector
            printTri(stl_stream, face);
        }
        stl_stream_.close();
        
    }

    TriFloatXYZ *subtractTwoPoints(TriFloatXYZ *P1, TriFloatXYZ *P2){
        TriFloatXYZ *resultPoint = new TriFloatXYZ;
        resultPoint->X = P1->X - P2->X;
        resultPoint->Y = P1->Y - P2->Y;
        resultPoint->Z = P1->Z - P2->Z;
        return resultPoint;
    }

    TriFloatXYZ *crossProductUnit(TriFloatXYZ *V1, TriFloatXYZ *V2){
        TriFloatXYZ *resultVector = new TriFloatXYZ;
        resultVector->X = (V1->Y * V2->Z) - (V1->Z*V2->Y); //i
        resultVector->Y = (V1->Z * V2->X) - (V1->X*V2->Z); //j
        resultVector->Z = (V1->X * V2->Y) - (V1->Y*V2->X); //k
        float magnitude = resultVector->X*resultVector->X + resultVector->Y*resultVector->Y;
        magnitude += resultVector->Z*resultVector->Z;
        magnitude = sqrt(magnitude);
        resultVector->X = resultVector->X/magnitude;
        resultVector->Y = resultVector->Y/magnitude;
        resultVector->Z = resultVector->Z/magnitude;

        return resultVector;
    }

private:
    std::ofstream stl_stream_; //binary stream to stl file
    std::vector<Triangle*> faces = {};
    std::vector<TriFloatXYZ*> vertices = {};

    void printBuffer(std::ofstream& stream){
        char bufInit = 0;
        //initiallize null 80 byte header
        for(int i=0; i<80; i++){
            stream.write((char*)&bufInit,1);
        }
    }

    void printNumTri(std::ofstream& stream){
        uint32_t triangleCount = faces.size();
        stream.write((char*)&triangleCount, sizeof(triangleCount));
    }

    void printTri(std::ofstream& stream, const Triangle *tri){
        //write normal to stream
        stream.write((char*)&tri->normal->X, 4);
        stream.write((char*)&tri->normal->Y, 4);
        stream.write((char*)&tri->normal->Z, 4);

        //write point1 to stream
        stream.write((char*)&tri->P1->X, 4);
        stream.write((char*)&tri->P1->Y, 4);
        stream.write((char*)&tri->P1->Z, 4);

        //write point2 to stream
        stream.write((char*)&tri->P2->X, 4);
        stream.write((char*)&tri->P2->Y, 4);
        stream.write((char*)&tri->P2->Z, 4);

        //write point3 to stream
        stream.write((char*)&tri->P3->X, 4);
        stream.write((char*)&tri->P3->Y, 4);
        stream.write((char*)&tri->P3->Z, 4);

        //attribute byte count (always 0)
        uint16_t byteC = 0; //attribute byte count
        stream.write((char*)&byteC, sizeof(byteC));
    }
    
    /*
    @brief modifies the unit vector of a triangle defined by 3 points
    */
    void normalUnitVector(Triangle* triangle){
        
        TriFloatXYZ *VectorP1P2 = subtractTwoPoints(triangle->P1, triangle->P2); 
        TriFloatXYZ *VectorP1P3 = subtractTwoPoints(triangle->P1, triangle->P3);
        
        triangle -> normal = crossProductUnit(VectorP1P2, VectorP1P3);

        delete VectorP1P2;
        delete VectorP1P3;
    }
}; //end STL_Binary Class

// MAIN *********************************************
//Creates basic curve, optimizes curve, creates top, bottom, sides, and back.
int main(){
    std::cout << "Making an optimized curved backboard.\n";
    
    //all units are cm for simplicity
    //defines back square of backboard
    float backHeight = 17;
    float backWidth = 34;
    float dVert = backHeight/numDivVert;
    float dHoriz = backWidth/numDivHoriz;
    
    //defines min thickness of backboard
    float backDepth = 3;

    STL_Binary BackBoard;

    //C for corner, used as temp variables
    TriFloatXYZ C1; //Bottom Left
    TriFloatXYZ C2; //Top Left
    TriFloatXYZ C3; //Top Right
    TriFloatXYZ C4; //Bottom Right

    //create initial curved front of backboard
    std::cout << "Create Curved Front.\n";
    float x_alpha = 0.2/backHeight; //defines parabolic shape of curve in horizontal direction
    float z_alpha = 0.2/backHeight; //defines parabolic shape of curve in vertical direction

    //store triangles
    Triangle* tri1;
    Triangle* tri2;
    Triangle* curveTopTris[numDivHoriz];
    Triangle* curveBottomTris[numDivHoriz];
    Triangle* curveLeftTris[numDivVert];
    Triangle* curveRightTris[numDivVert];
    Triangle* curvedFrontTris[numDivVert][numDivHoriz*2]; //two triangles per division

    int halfVert = numDivVert/2;
    int halfHoriz = numDivHoriz/2;
    float x_adjust = halfHoriz*dHoriz;
    float z_adjust = halfVert*dVert;
    float y_adjust = backDepth;

    //iterate vertically through squares
    for(int i= (-halfVert); i < halfVert; i++){
        //iterate horizontally through squares
        for(int j=(-halfHoriz); j<halfHoriz; j++){
            //get all 4 points of each square C1,C2,C3,C4
            //each square needs X, Y to be centered on center of back for parabola equation
            C1.X = dHoriz*j;
            C1.Z = dVert*i;
            C1.Y = x_alpha*C1.X*C1.X + z_alpha*C1.Z*C1.Z; //defines parabola shape
            //shifts the curve back so that it is in corect postion relative to back of backboard
            C1.X += x_adjust; 
            C1.Z += z_adjust;
            C1.Y += y_adjust;

            C2.X = dHoriz*j;
            C2.Z = dVert*(i+1);
            C2.Y = x_alpha*C2.X*C2.X + z_alpha*C2.Z*C2.Z;
            C2.X += x_adjust;
            C2.Z += z_adjust;
            C2.Y += y_adjust;

            C3.X = dHoriz*(j+1);
            C3.Z = dVert*(i+1);
            C3.Y = x_alpha*C3.X*C3.X + z_alpha*C3.Z*C3.Z;
            C3.X += x_adjust;
            C3.Z += z_adjust;
            C3.Y += y_adjust;

            C4.X = dHoriz*(j+1);
            C4.Z = dVert*i;
            C4.Y = x_alpha*C4.X*C4.X + z_alpha*C4.Z*C4.Z;
            C4.X += x_adjust;
            C4.Z += z_adjust;
            C4.Y += y_adjust;
            
            //add square to curve as two triangles
            tri1= BackBoard.addTriangle(&C1, &C2, &C3);
            tri2= BackBoard.addTriangle(&C1, &C3, &C4);

            // useful to find individual faces for testing
            // if(i==(halfVert-1) && j==(-halfHoriz)){
            //     std::cout<<"Specific Face.";
            // }
            
            //store all triangle faces
            curvedFrontTris[i+halfVert][2*(j+halfHoriz)] = tri1;
            curvedFrontTris[i+halfVert][2*(j+halfHoriz)+1] = tri2;

            //if bottom triangle row store tri pointer to array
            if(i == -(numDivVert/2)){
                curveBottomTris[j+halfHoriz] = tri2; //adjust j to be 0-numDivVert
            }

            //if top triangle row store tri pointer to array
            if(i == (numDivVert/2-1)){
                curveTopTris[j+halfHoriz] = tri1; //adjust j to be 0-numDivVert
            }

            //if left triangle row store tri pointer to array
            if(j == -(numDivHoriz/2)){
                curveLeftTris[i+halfVert] = tri1; //adjust i to be 0-numDivHoriz
            }
            //if right triangle row store tri pointer to array
            if(j == (numDivHoriz/2-1)){
                curveRightTris[i+halfVert] = tri2; //adjust i to be 0-numDivHoriz
            }

        }
    }

    //run the optimize function. 
    //In the future can run multiple times to get a reasonable level of optimization
    float avgWidthHeight = (dVert+dHoriz)/2;
    for(int i=0; i<numOptimizations; i++){
        std::cout<< "Running Optimization Cycle "<< (i+1) << "/" << numOptimizations << "\n";
        optimizeBackboardCurve(curvedFrontTris, avgWidthHeight);
        std::cout<< "***************************************************\n\n";
    }

    //add top face
    std::cout << "Create Top.\n";
    C1.Z = backHeight;
    C2.Z = backHeight;
    C3.Z = backHeight;
    C4.Z = backHeight;
    C1.Y = 0;
    C2.Y = 0;
    
    //create top of backboard
    for(int i=0; i<numDivHoriz; i++){
        C1.X = i*dHoriz;
        C2.X = (i+1)*dHoriz;
        C3.X = (i+1)*dHoriz;
        C4.X = i*dHoriz;

        //use stored array to get Y value of top row of triangles
        C3.Y = curveTopTris[i]->P3->Y;
        C4.Y = curveTopTris[i]->P2->Y;

        //add triangle
        BackBoard.addTriangle(&C1, &C2, &C3);
        BackBoard.addTriangle(&C1, &C3, &C4);
    }

    //Add Bottom Face
    std::cout << "Create Bottom.\n";
    C1.Z = 0;
    C2.Z = 0;
    C3.Z = 0;
    C4.Z = 0;
    C1.Y = 0;
    C2.Y = 0;
    
    for(int i=0; i<numDivHoriz; i++){
        C1.X = i*dHoriz;
        C2.X = (i+1)*dHoriz;
        C3.X = (i+1)*dHoriz;
        C4.X = i*dHoriz;

        //use stored array to get Y value of top row of triangles
        C3.Y = curveBottomTris[i]->P3->Y;
        C4.Y = curveBottomTris[i]->P1->Y;

        //add triangle
        BackBoard.addTriangle(&C3, &C2, &C1);
        BackBoard.addTriangle(&C4, &C3, &C1);
    }

    //add left face
    std::cout << "Create Left.\n";
    C1.X = 0;
    C2.X = 0;
    C3.X = 0;
    C4.X = 0;
    C1.Y = 0;
    C2.Y = 0;
    
    for(int i=0; i<numDivVert; i++){
        C1.Z = i*dVert;
        C2.Z = (i+1)*dVert;
        C3.Z = (i+1)*dVert;
        C4.Z = i*dVert;

        //use stored array to get Y value of top row of triangles
        C3.Y = curveLeftTris[i]->P2->Y;
        C4.Y = curveLeftTris[i]->P1->Y;

        //add triangle
        BackBoard.addTriangle(&C1, &C2, &C3);
        BackBoard.addTriangle(&C1, &C3, &C4);
    }

    //add right face
    std::cout << "Create Right.\n";
    C1.X = backWidth;
    C2.X = backWidth;
    C3.X = backWidth;
    C4.X = backWidth;
    C1.Y = 0;
    C2.Y = 0;
    
    for(int i=0; i<numDivVert; i++){
        C1.Z = i*dVert;
        C2.Z = (i+1)*dVert;
        C3.Z = (i+1)*dVert;
        C4.Z = i*dVert;

        //use stored array to get Y value of top row of triangles
        C3.Y = curveRightTris[i]->P2->Y;
        C4.Y = curveRightTris[i]->P3->Y;

        //add triangle
        BackBoard.addTriangle(&C3, &C2, &C1);
        BackBoard.addTriangle(&C4, &C3, &C1);
    }

    //Add Back
    std::cout << "Create Back.\n";
    C1.Y = 0;
    C2.Y = 0;
    C3.Y = 0;
    C4.Y = 0;

    for(int i=0; i<numDivVert; i++){
        for(int j=0; j<numDivHoriz; j++){
            C1.X = j*dHoriz;
            C1.Z = i*dVert;

            C2.X = (j+1)*dHoriz;
            C2.Z = i*dVert;

            C3.X = (j+1)*dHoriz;
            C3.Z = (i+1)*dVert;

            C4.X = j*dHoriz;
            C4.Z = (i+1)*dVert;

            BackBoard.addTriangle(&C1, &C2, &C3);
            BackBoard.addTriangle(&C1, &C3, &C4);
        }
    }

    //Render STL
    std::cout << "Generated with " << BackBoard.numTriangles() << " faces." <<std::endl;
    std::cout << "Rendering STL.\n";
    BackBoard.renderSTL("OptimizedBackBoard.stl");

    std::cout << "Program Completed Successfully.";
    return 0;
}


// Optimization Function *********************************************
// This makes small changes to each face, then runs tests to see if the face has improved.
// Each test on a face is run in a seperate thread to improve CPU utilization.
void optimizeBackboardCurve(Triangle* curvedFrontTris[numDivVert][numDivHoriz*2], float avgWidthHeight){
    std::cout << "Optimizing Curved Front...\n";

    //face: a square that matches up with the number of vert and horiz divisions
    //every face has two triangles, and 4 adjustable points

    //random seed
    srand(static_cast<unsigned int>(time(0)));

    float averageTime=0;
    long int secondsToGo = 0;
    int percentTracker=0;

    //two triangles of face
    Triangle *curFaceTri1;
    Triangle *curFaceTri2;

    //for corners of face
    TriFloatXYZ *C1;
    TriFloatXYZ *C2;
    TriFloatXYZ *C3;
    TriFloatXYZ *C4;

    float maxPointDeviation = (0.25)*avgWidthHeight;
    int randAdjP1;
    int randAdjP2;
    int randAdjP3;
    int randAdjP4;

    const int numTestsPerFace = 20; 
    const int maxActiveThreads = 20; 
    int numCurrentThreads=0;
    int mostRecentActiveThread;
    faceTest faceTests[numTestsPerFace];
    std::vector<std::thread> faceThreads;
    // Source: https://cplusplus.com/reference/thread/thread/thread/

        //loop through all faces
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
        row= faceID / numDivHoriz;
        col= faceID % numDivHoriz;
        curFaceTri1 = curvedFrontTris[row][col];
        curFaceTri2 = curvedFrontTris[row][col+1];

        C1 = curFaceTri1->P1;
        C2 = curFaceTri1->P2;
        C3 = curFaceTri1->P3;
        C4 = curFaceTri2->P3;

        //set up tests
        for(int j=0 ; j<numTestsPerFace; j++){
            copyPoint(&faceTests[j].P1, C1);
            copyPoint(&faceTests[j].P2, C2);
            copyPoint(&faceTests[j].P3, C3);
            copyPoint(&faceTests[j].P4, C4);
            faceTests[j].performance = 0;
            faceTests[j].testCompleted = false;
        }

        numCurrentThreads = 1;
        //test with no changes (except points that were changed by other faces)
        faceThreads.push_back(std::thread(testFace, &faceTests[0]));
        
        int randFace;
        //test the face under varying conditions
        for(int j=1 ; j<numTestsPerFace; j++){
            //randomly select a corner, then randomly adjust that corner (different random adjustment for each test)
            randFace = rand()%4;
            if(randFace == 0){
                randAdjP1 = rand()%200 -100; //-100 - 99
                faceTests[j].P1.Y += ((float)randAdjP1 /100.0)*maxPointDeviation;
            }
            else if(randFace==1){
                randAdjP2 = rand()%200 -100;
                faceTests[j].P2.Y += ((float)randAdjP2 /100.0)*maxPointDeviation;
            }
            else if(randFace==2){
                randAdjP3 = rand()%200 -100;
                faceTests[j].P3.Y += ((float)randAdjP3 /100.0)*maxPointDeviation;
            }
            else if(randFace==3){
                randAdjP4 = rand()%200 -100;
                faceTests[j].P4.Y += ((float)randAdjP4 /100.0)*maxPointDeviation;
            }

            //join first created and active thread if too many threads are running
            if(numCurrentThreads >= maxActiveThreads){
                mostRecentActiveThread = i*numTestsPerFace+(j-maxActiveThreads);
                if(faceThreads.at(mostRecentActiveThread).joinable()){
                    faceThreads.at(mostRecentActiveThread).join(); 
                }
                numCurrentThreads--;
            }

            numCurrentThreads++;
            faceThreads.push_back(std::thread(testFace, &faceTests[j]));
        }

        //join all threads, check each thread to see if it's joinable
        //Source: https://stackoverflow.com/questions/38412471/c-join-a-vector-of-threads
        for(unsigned int j=0; j<numTestsPerFace; j++){
            if(faceThreads.at(i*numTestsPerFace+j).joinable()){
                faceThreads.at(i*numTestsPerFace+j).join();
            }
        }
        
        //determine the best version
        int bestVersion = 0; //current version
        float bestPerformance = 0;
        for(int j=0; j<numTestsPerFace; j++){
            if(faceTests[j].performance > FLOAT_E){ //if it's not 0
                if(faceTests[j].performance > bestPerformance){
                    bestVersion = j;
                }
            }
        }

        //output program expected time
        //get average time
        averageTime=0;
        for(int j=0; j<numTestsPerFace; j++){
            averageTime += faceTests[j].time;
        }
        averageTime /= numTestsPerFace;
        secondsToGo = (long int)((numFaces-i)*averageTime/1e03f);
        if(i!=0){
            //print out the time if 5% has happened
            if((int)(((float)i/numFaces)*100) - percentTracker > 5){
                percentTracker+=5;
                std::cout << percentTracker << "% completed. " << "Time Left: " << (secondsToGo/60/60);
                std::cout << " hours, " << (secondsToGo/60)%60 << "minutes, ";
                std::cout << secondsToGo%60 << " seconds.\n";
            }
        } //first cycle complete print time prediction
        else{
            std::cout << "0% completed. " << "Time Left: " << (secondsToGo/60/60);
            std::cout << " hours, " << (secondsToGo/60)%60 << " minutes, ";
            std::cout << secondsToGo%60 << " seconds.\n";
        }

        //set the actual curve to be the best performing face.
        curFaceTri1->P1->Y = faceTests[bestVersion].P1.Y;
        curFaceTri1->P2->Y = faceTests[bestVersion].P2.Y;
        curFaceTri1->P3->Y = faceTests[bestVersion].P3.Y;
        curFaceTri2->P3->Y = faceTests[bestVersion].P4.Y;
    }
    std::cout << "Optimization Complete.\n";
}

//Constants for Single Face Test*********************************************
const int maxNumExceptions = 10; //max num exceptions that occur when finding trajectory
//shooter area: all area that a shot may occur from
//target: target point for a ball to hit 

//defines parameters for target relative to origin
const float g_a = 980.665; //cm/s^2
const float ballRadius = 5.08;
const float ballBounceRestitution = 0.5; //how bouncy the ball is
const float targetY = 15-ballRadius;
const TriFloatXYZ targetPoint = {11.5f, targetY, -1.5f};
const float maxTargetDis = 4; //max distance from target that will likely still go in the hoop
const float maxTargetDisSquared = maxTargetDis*maxTargetDis;

const float dShooter = 2; //defines the iterating size in cm over the shooter area

//based on typical height of an individual, shooting just over the head
const float shootHeightMin = 155; //5ft
const float shootHeightMax = 200; //6.5 ft

const float maxArcHeight = 245; //around 8ft or the typical ceiling height

// minAcceptablemaxHOffset means that the parabolic arc of a target must at least 
// have a curve that deviates by 5cm vertically. 
// This also bounds the max speed the ball can be thrown.
const float minAcceptablemaxHOffset = 10; 

const float minShootDistance = 50; //min distance from target, 1.6 ft
const float shootBoxWidth = 40; //shooter box width
const float shootBoxDepth = 40; //shooter box depth
const float shootBoxHeight = shootHeightMax-shootHeightMin;
const float shootBoxXStart = targetPoint.X - shootBoxWidth/2;

const int shootBoxXDiv = (int)(shootBoxWidth/dShooter);
const int shootBoxYDiv = (int)(shootBoxDepth/dShooter);
const int shootBoxZDiv = (int)(shootBoxHeight/dShooter);
const int maxHDiv      = (int)((maxArcHeight-shootHeightMin)/dShooter);
const double numShootPos  = (double)shootBoxXDiv*(double)shootBoxYDiv*(double)shootBoxZDiv*(double)maxHDiv; //this is an upper bound, not a true value

//tests a single face defined in the faceTest structure
//designed so that it only access faceTest structure and can consequentially run in a seperate thread
void testFace(void* faceIn){
    faceTest* face = (faceTest*)faceIn;

    auto start = std::chrono::high_resolution_clock::now();
    int numExceptions = 0;
    int numTargetHits = 0;
    TriFloatXYZ ballEnd;
    ballEnd.Z = targetPoint.Z; //always the same
    TriFloatXYZ bouncePoint;
    TriFloatXYZ ballStart;
    TriFloatXYZ faceNV; //normal unit vector to face

    float maxH=0;
    float minAcceptableMaxHeight;
    float ballDis;

    Triangle faceNormal = {&faceNV, &(face->P1), &(face->P2), &(face->P3)};
    getNormal(&faceNormal);

    //get center point of face
    bouncePoint.X = (face->P1.X + face->P2.X + face->P3.X + face->P4.X)/4; 
    bouncePoint.Y = (face->P1.Y + face->P2.Y + face->P3.Y + face->P4.Y)/4; 
    bouncePoint.Z = (face->P1.Z + face->P2.Z + face->P3.Z + face->P4.Z)/4; 

    //iterate through all shooting positions
    for(int i=(-shootBoxXDiv/2); i<(shootBoxXDiv/2); i++){
        for(int j=(-shootBoxYDiv/2); j<(shootBoxYDiv/2); j++){
            for(int k=0; k<shootBoxZDiv; k++){
                //determine ball starting point
                ballStart.X = i*dShooter + shootBoxXStart;
                ballStart.Y = j*dShooter + minShootDistance;
                ballStart.Z = k*dShooter + shootHeightMin;

                //ensures that the maxH the ball reaches still follows a parabolic arc
                minAcceptableMaxHeight = (bouncePoint.Z-ballStart.Z)/2 + ballStart.Z + minAcceptablemaxHOffset;

                for(int l=0; l<maxHDiv; l++){
                    maxH = l*dShooter + shootHeightMin;
                    if(maxH < minAcceptableMaxHeight){
                        continue;
                    }
                    //try to find a target. As this happens a lot (and math erros could happen), a simple try-except block was added.
                    try {
                        findTrajectory(&ballEnd, &bouncePoint, &ballStart, &faceNV, maxH);
                    }
                    catch(...){
                        numExceptions++;
                        std::cout << "Exception occured when finding trajectory. " << numExceptions << " have occured.";
                        if(numExceptions > maxNumExceptions){
                            std::cout << "Max Exceptions occured. Program exiting.";
                            exit(1);
                        }
                        continue; //don't test against the target
                    }
                    //test if ballEnd is within exceptable range of target
                    ballDis = fabs(ballEnd.X-targetPoint.X) + fabs(ballEnd.Y-targetPoint.Y);

                    //if target was hit
                    if(ballDis < maxTargetDisSquared){
                        numTargetHits++;
                    }
                }
            }
        }
    }

    face->performance = ((float)numTargetHits)/numShootPos; //percentage of successful shots

    auto stop = std::chrono::high_resolution_clock::now();
    
    //calculate the duration it took
    face->time = (int)(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)).count();
    face->testCompleted = true;
}

//FINDING TRAJECTORY **************************************************************************************************************
//created externally to avoid creating a bunch of times
TriFloatXYZ v2; //vector before bounce
TriFloatXYZ v3; //vector after bounce

float dt1; //start point to pounce point 
float dt2; //maxh to bounce point
float dt3; //bounce point to end point
float bounceProduct; //=(1+e)(v2⋅vn) scalar defined to help calculate vector bounce of ball
float dz; //height change from bounce point to ballEnd
float maxH2;

//finds the ball end given parameters
void findTrajectory(TriFloatXYZ* ballEnd, TriFloatXYZ* bouncePoint, TriFloatXYZ* ballStart, TriFloatXYZ* faceNV, float maxH){
    //solve for v2, speed vector before the bounce, first solve for dt1, time it takes to move from start point to bounce point
    dt2 = sqrt(2*(maxH-bouncePoint->Z)/g_a);
    dt1 = sqrt(2*(maxH-ballStart->Z)/g_a) + dt2;
    v2.Z = (maxH-bouncePoint->Z)/dt2;
    v2.X = (bouncePoint->X-ballStart->X)/dt1;
    v2.Y = (bouncePoint->Y-ballStart->Y)/dt1;

    //calculate the bounce vector
    //v3=v2 - (1+e)(v2⋅vn)vn
    bounceProduct = (v2.X*faceNV->X + v2.Y*faceNV->Y + v2.Z*faceNV->Z);
    bounceProduct *= (1+ballBounceRestitution);
    v3.X = v2.X - bounceProduct*faceNV->X;
    v3.Y = v2.Y - bounceProduct*faceNV->Y;
    v3.Z = v2.Z - bounceProduct*faceNV->Z;

    //calculate the end position, two posabilities, bounces down, or bounces up
    //bounces up (arcs up then back down)
    if(v3.Z > FLOAT_E){ //compare to 0, float E ensures v3.Z isn't tiny
        maxH2 = bouncePoint->Z + (v3.Z*v3.Z)/(2*g_a); 
        dt3 = sqrt(2*(maxH2-bouncePoint->Z)/g_a) + sqrt(2*(maxH2-targetPoint.Z)/g_a);
        ballEnd->X = v2.X*dt3 + bouncePoint->X;
        ballEnd->Y = v2.Y*dt3 + bouncePoint->Y;
    }//bounces down (only arcs down)
    else{
        dt3 = (v3.Z - sqrt(v3.Z*v3.Z - g_a*(bouncePoint->Z-targetPoint.Z))) / (-g_a);
        ballEnd->X = v2.X*dt3 + bouncePoint->X;
        ballEnd->Y = v2.Y*dt3 + bouncePoint->Y;
    }   
    
}