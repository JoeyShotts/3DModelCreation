/*
Joseph Shotts
10/8/2025
C++ Code
Description: 
Creating a simple curved backboard.
*/

#include <fstream> // Required for file stream operations
#include <iostream>
#include <cmath>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <time.h>

#define FLOAT_E (float)1e-09 //used for float comparison

struct TriFloatXYZ{
    float X=0;
    float Y=0;
    float Z=0;
};

struct Triangle{
    TriFloatXYZ *normal;
    TriFloatXYZ *P1;
    TriFloatXYZ *P2;
    TriFloatXYZ *P3;
};

//for new point
TriFloatXYZ *nP(float X, float Y, float Z){
    TriFloatXYZ *point = new TriFloatXYZ;
    point->X = X;
    point->Y = Y;
    point->Z = Z;
    return point;
}

void copyPoint(TriFloatXYZ *copiedPoint, TriFloatXYZ *origPoint){
    copiedPoint->X=origPoint->X;
    copiedPoint->Y=origPoint->Y;
    copiedPoint->Z=origPoint->Z;
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
        for (const Triangle* face : faces) {
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

#define numDivVert (int)60 //must be even
#define numDivHoriz (int)120 //must be even

void optimizeBackboardCurve(Triangle* curvedFrontTris[numDivVert][numDivHoriz*2]);

int main(){
    std::cout << "Making a simple curved backboard.\n";
    
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

    optimizeBackboardCurve(curvedFrontTris);

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
    std::cout << "Rendering STL.";
    BackBoard.renderSTL("BackBoard.stl");
    std::cout << "Program Completed Successfully.";
    return 0;
}

void optimizeBackboardCurve(Triangle* curvedFrontTris[numDivVert][numDivHoriz*2]){
    std::cout << "Optimizing Curved Front...";

    // curvedFrontTris[1][1]->P1->Y += 1; //test

    //shooter area: all area that a shot may occur from
    //target: target point for a ball to hit 
    //face: a square that matches up with the number of vert and horiz divisions
    //every face has two triangles, and 4 adjustable points

    //defines parameters for target relative to origin
    TriFloatXYZ targetPoint = {11.09, 15, -1.5};

    float dShooter = 1; //defines the iterating size in cm over the shooter area

    //based on typical height of an individual, shooting just over the head
    float shootHeightMin = 155; //5ft
    float shootHeightMax = 200; //6.5 ft

    float maxArcHeight = 245; //around 8ft or the typical ceiling height
    
    float minShootDistance = 50; //min distance from target, 1.6 ft
    float shootBoxWidth = 200; //shooter box width
    float shootBoxDepth = 200; //shooter box depth

    //random seed
    srand(static_cast<unsigned int>(time(0)));

    //loop through all faces
    int numFaces = numDivHoriz*numDivVert; 
    bool wasFaceChanged[numFaces];
    int faceID; //integer divide by numDivHoriz to get row(0-numDivVert), modulo numDivHoriz to get column(0-numDivHoriz)

    //two triangles of face
    Triangle *curFaceTri1;
    Triangle *curFaceTri2;

    //for corners of face
    TriFloatXYZ *C1;
    TriFloatXYZ *C2;
    TriFloatXYZ *C3;
    TriFloatXYZ *C4;

    int row, col;

    //reset wasFaceChanged for all faces
    for(int i=0; i<numFaces;i++){
        wasFaceChanged[i] = false;
    }
    for(int i=0; i<numFaces; i++){
        faceID = i;
        faceID = rand() % numFaces;
        //find a new face if a face has already been changed
        while(wasFaceChanged[faceID]){
            faceID++;
            faceID %= numFaces;
        }
        wasFaceChanged[faceID] = true;
        
        //get current face
        row= faceID/numDivHoriz;
        col= faceID % (2*numDivHoriz);
        curFaceTri1 = curvedFrontTris[row][col];
        curFaceTri2 = curvedFrontTris[row][col+1];

        C1 = curFaceTri1->P1;
        C2 = curFaceTri1->P2;
        C3 = curFaceTri1->P3;
        C4 = curFaceTri2->P3;

        // //test to see if I'm accesses all points correctly
        // C1->Y += 0.1;
        // C2->Y += 0.1;
        // C3->Y += 0.1;
        // C4->Y += 0.1;
    }
}

//seg fault at > 7170