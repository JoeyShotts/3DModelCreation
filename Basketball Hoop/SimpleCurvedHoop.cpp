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
#include <windows.h>

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
        newFace->P1 = new TriFloatXYZ;
        newFace->P2 = new TriFloatXYZ;
        newFace->P3 = new TriFloatXYZ;

        copyPoint(newFace->P1, P1);
        copyPoint(newFace->P2, P2);
        copyPoint(newFace->P3, P3);

        normalUnitVector(newFace);
        faces.push_back(newFace);
        return newFace;
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

int main(){
    std::cout << "Making a simple curved backboard.\n";
    
    //defines back square of backboard
    float backHeight = 17;
    float backWidth = 34;
    int numDivVert = 60; //must be even
    int numDivHoriz = 120; //must be even
    float dVert = backHeight/numDivVert;
    float dHoriz = backWidth/numDivHoriz;
    
    //defines min thickness of backboard
    float backDepth = 5;

    STL_Binary BackBoard;

    //create back
    TriFloatXYZ Origin = {0,0,0};
    TriFloatXYZ TopLeft = {0,0,backHeight};
    TriFloatXYZ TopRight = {backWidth,0,backHeight};
    TriFloatXYZ BottomRight = {backWidth,0,0};

    BackBoard.addTriangle(&TopRight, &TopLeft, &Origin);
    BackBoard.addTriangle(&BottomRight, &TopRight, &Origin);

    //create initial curved front of backboard
    //C for corner
    TriFloatXYZ C1; //Bottom Left
    TriFloatXYZ C2; //Top Left
    TriFloatXYZ C3; //Top Right
    TriFloatXYZ C4; //Bottom Right

    float x_alpha = 0.2/backHeight; //defines parabolic shape of curve in horizontal direction
    float z_alpha = 0.2/backHeight; //defines parabolic shape of curve in vertical direction
    float x_adjust = (numDivHoriz/2)*dHoriz;
    float z_adjust = (numDivVert/2)*dVert;
    float y_adjust = backDepth;

    //store top row triangles
    Triangle* tri1;
    Triangle* tri2;
    Triangle* curveTopTris[numDivHoriz]; 

    //iterate vertically through squares
    for(int i=(-numDivVert/2); i<numDivVert/2; i++){
        //iterate horizontally through squares
        for(int j=(-numDivHoriz/2); j<numDivHoriz/2; j++){
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

            //if top triangle row store tri pointer to array
            if(i == (numDivVert/2-1)){
                curveTopTris[j+numDivHoriz/2] = tri1; //adjust i to be 0-numDivVert
            }
        }
    }

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

    std::cout << "Generated with " << BackBoard.numTriangles() << " faces." <<std::endl;

    BackBoard.renderSTL("BackBoard.stl");
    
    Sleep(1000); //just lets me read the output to the terminal in VS code

    return 0;
}
