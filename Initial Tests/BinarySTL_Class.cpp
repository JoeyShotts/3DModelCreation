/*
Joseph Shotts
10/8/2025
C++ Code
Description: In progress code to turn a set of binary stl creation functions into a class.
*/

#include <fstream> // Required for file stream operations
#include <iostream>
#include <cmath>
#include <cstdint>
#include <vector>

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

class STL_Binary{
public:
    STL_Binary(const std::string& name):
    stl_stream_(name, std::ios::out | std::ios::binary){;}
    
    void addTriangle(TriFloatXYZ *P1, TriFloatXYZ *P2, TriFloatXYZ *P3){
        Triangle *newFace = new Triangle;
        newFace->P1 = P1;
        newFace->P2 = P2;
        newFace->P3 = P3;
        normalUnitVector(newFace);
        faces.push_back(newFace);

    }

    void renderSTL(){
        printBuffer();
        printNumTri();
        for (const Triangle* face : faces) {
            printTri(face);
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
    uint32_t triangleCount = 0;

    void printBuffer(){
        char bufInit = 0;
        //initiallize null 80 byte header
        for(int i=0; i<80; i++){
            stl_stream_.write((char*)&bufInit,1);
        }
    }

    void printNumTri(){
         stl_stream_.write((char*)&triangleCount, sizeof(triangleCount));
    }

    void printTri(const Triangle *tri){
        //write normal to stream
        stl_stream_.write((char*)&tri->normal->X, 4);
        stl_stream_.write((char*)&tri->normal->Y, 4);
        stl_stream_.write((char*)&tri->normal->Z, 4);

        //write point1 to stl_stream_
        stl_stream_.write((char*)&tri->P1->X, 4);
        stl_stream_.write((char*)&tri->P1->Y, 4);
        stl_stream_.write((char*)&tri->P1->Z, 4);

        //write point2 to stl_stream_
        stl_stream_.write((char*)&tri->P2->X, 4);
        stl_stream_.write((char*)&tri->P2->Y, 4);
        stl_stream_.write((char*)&tri->P2->Z, 4);

        //write point3 to stl_stream_
        stl_stream_.write((char*)&tri->P3->X, 4);
        stl_stream_.write((char*)&tri->P3->Y, 4);
        stl_stream_.write((char*)&tri->P3->Z, 4);

        //attribute byte count (always 0)
        uint16_t byteC = 0; //attribute byte count
        stl_stream_.write((char*)&byteC, sizeof(byteC));
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
    std::cout << "Empty C++ File.";
    
    STL_Binary simplePyramid("BinrarySTL_ClassTest.stl");

    //defining a 6 faced triangle pyramid (two on bottom)
    TriFloatXYZ *tip = nP(1,1,2);

    //Bottom1
    simplePyramid.addTriangle(nP(0,0,0), nP(0,2,0), nP(2,0,0));

    //Bottom2
    simplePyramid.addTriangle(nP(0,2,0), nP(2,2,0), nP(2,0,0));

    //Side1
    simplePyramid.addTriangle(tip, nP(0,0,0), nP(2,0,0));

    //Side2
    simplePyramid.addTriangle(tip, nP(2,0,0), nP(2,2,0));

    //Side3
    simplePyramid.addTriangle(tip, nP(2,2,0), nP(0,2,0));

    //Side4
    simplePyramid.addTriangle(tip, nP(0,2,0), nP(0,0,0));

    simplePyramid.renderSTL();

    return 0;
}
