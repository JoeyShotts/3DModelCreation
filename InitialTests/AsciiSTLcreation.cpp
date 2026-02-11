/*
Joseph Shotts
10/8/2025
C++ Code
Description: First .stl file test. Creates a Pyramid in Ascii Format.
Created with help from: https://en.wikipedia.org/wiki/STL_(file_format)
*/

#include <fstream> // Required for file stream operations
#include <iostream>
#include <cmath>
#include <cstdint>
#include <string>
#include <iomanip>

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

void printName(std::ofstream &stream, const std::string &name){
    stream << "solid " << name << std::endl;
}

void printEnd(std::ofstream &stream, const std::string &name){
    stream << "endsolid " << name << std::endl;
}

void printTri(std::ofstream& stream, Triangle *tri){
    stream<<"facet normal "
        <<tri->normal->X<<" "<<tri->normal->Y<<" "<<tri->normal->Z<<" "<<std::endl;
    stream<<"\touter loop"<<std::endl;
    stream<<"\t\t"<<"vertex "<<
        tri->P1->X<<" "<<tri->P1->Y<<" "<<tri->P1->Z<<" "<<std::endl;
    stream<<"\t\t"<<"vertex "<<
        tri->P2->X<<" "<<tri->P2->Y<<" "<<tri->P2->Z<<" "<<std::endl;
    stream<<"\t\t"<<"vertex "<<
        tri->P3->X<<" "<<tri->P3->Y<<" "<<tri->P3->Z<<" "<<std::endl;
    stream<<"\tendloop"<<std::endl;
    stream<<"endfacet"<<std::endl;
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

    std::ofstream outFile;
    outFile.open("SimpleAscii.stl");
    
    //format the floating point numbers so they will print correctly
    outFile
        << std::scientific    // Use scientific notation (e.g., 1.23e+02)
        << std::showpos       // Show '+' for positive exponent AND positive number
        << std::setprecision(6) // Set 6 digits after the decimal point
        << std::uppercase;    // Use 'E' instead of 'e' for the exponent

    if (!outFile.is_open()) {
        std::cerr << "Error: Unable to open file for writing." << std::endl;
        return 1; // Indicate an error
    }

    std::string name = "SimpleAscii";
    printName(outFile, name);

    printTri(outFile, &Bottom1);
    printTri(outFile, &Bottom2);
    printTri(outFile, &Side1);
    printTri(outFile, &Side2);
    printTri(outFile, &Side3);
    printTri(outFile, &Side4);

    printEnd(outFile, name);

    outFile.close();

    return 0;
}
