/*
Joseph Shotts
10/8/2025
C++ Code
Description: First .stl file test.
*/

#include <fstream> // Required for file stream operations
#include <iostream>
#include <cmath>
#include <cstdint>

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

//for new point
TriFloatXYZ *nP(float X, float Y, float Z){
    TriFloatXYZ *point = new TriFloatXYZ;
    point->X = X;
    point->Y = Y;
    point->Z = Z;
    return point;
}

void defTri(Triangle *newTri, TriFloatXYZ *P1, TriFloatXYZ *P2, TriFloatXYZ *P3){
    newTri->P1 = P1;
    newTri->P2 = P2;
    newTri->P3 = P3;
    
    normalUnitVector(newTri);
}

void printBuffer(std::ofstream &stream){
    char bufInit = 0;
    //initiallize null 80 byte header
    for(int i=0; i<80; i++){
        stream.write((char*)&bufInit,1);
    }
}

void printNumTri(std::ofstream& stream, uint32_t numTris){
    stream.write((char*)&numTris, sizeof(numTris));
}

void printPoint(std::ofstream& stream, TriFloatXYZ *P){
    stream.write((char*)&P->X, sizeof(P->X));
    stream.write((char*)&P->Y, sizeof(P->Y));
    stream.write((char*)&P->Z, sizeof(P->Z));

}

void printTri(std::ofstream& stream, Triangle *tri){
    uint16_t byteC = 0; //attribute byte count

    printPoint(stream, tri->normal);
    printPoint(stream, tri->P1);
    printPoint(stream, tri->P2);
    printPoint(stream, tri->P3);

    stream.write((char*)&byteC, sizeof(byteC));
}

int main(){
    std::cout << "Empty C++ File.";
    

    //defining a 6 faced triangle pyramid (two on bottom)
    TriFloatXYZ *tip = nP(1,1,2);

    Triangle Bottom1;
    defTri(&Bottom1, nP(0,0,0), nP(0,2,0), nP(2,0,0));

    Triangle Bottom2;
    defTri(&Bottom2, nP(0,2,0), nP(2,2,0), nP(2,0,0));

    Triangle Side1;
    defTri(&Side1, tip, nP(0,0,0), nP(2,0,0));

    Triangle Side2;
    defTri(&Side2, tip, nP(2,0,0), nP(2,2,0));

    Triangle Side3;
    defTri(&Side3, tip, nP(2,2,0), nP(0,2,0));

    Triangle Side4;
    defTri(&Side4, tip, nP(0,2,0), nP(0,0,0));

    std::ofstream outFile("SimpleSTL_Test.stl", std::ios::out | std::ios::binary);

    if (!outFile.is_open()) {
        std::cerr << "Error: Unable to open file for writing." << std::endl;
        return 1; // Indicate an error
    }

    printBuffer(outFile);
    printNumTri(outFile, 6);
    printTri(outFile, &Bottom1);
    printTri(outFile, &Bottom2);
    printTri(outFile, &Side1);
    printTri(outFile, &Side2);
    printTri(outFile, &Side3);
    printTri(outFile, &Side4);

    outFile.close();

    for(;;); //for debugging
    return 0;
}
