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
    void addTriangle(TriFloatXYZ *P1, TriFloatXYZ *P2, TriFloatXYZ *P3){
        Triangle *newFace = new Triangle;
        newFace->P1 = new TriFloatXYZ;
        newFace->P2 = new TriFloatXYZ;
        newFace->P3 = new TriFloatXYZ;

        copyPoint(newFace->P1, P1);
        copyPoint(newFace->P2, P2);
        copyPoint(newFace->P3, P3);

        normalUnitVector(newFace);
        faces.push_back(newFace);

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
    std::cout << "Making a simple curved backboard.";
    
    //defines back square of backboard
    float backHeight = 17;
    float backWidth = 34;
    int numDivVert = 50;
    int numDivHoriz = 80;
    
    //defines thickness of backboard
    float backDepth = 10;

    STL_Binary BackBoard;

    TriFloatXYZ Origin = {0,0,0};
    TriFloatXYZ TopLeft = {0,0,backHeight};
    TriFloatXYZ TopRight = {0,backWidth,backHeight};
    TriFloatXYZ BottomRight = {0,backWidth,0};

    BackBoard.addTriangle(&Origin, &TopLeft, &TopRight);
    BackBoard.addTriangle(&Origin, &TopRight, &BottomRight);


    std::cout << "Generated with " << BackBoard.numTriangles() << " faces." <<std::endl;

    BackBoard.renderSTL("BackBoard.stl");
    

    for(;;);


    return 0;
}
