/*
Joseph Shotts
10/8/2025
C++ Code
Description: In progress code to turn a set of binary stl creation functions into a class.
https://en.wikipedia.org/wiki/Sphericon
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


class STL_Binary{
public:
    TriFloatXYZ* copyPoint(TriFloatXYZ* origPoint){
        TriFloatXYZ* copiedPoint = new TriFloatXYZ;
        copiedPoint->X=origPoint->X;
        copiedPoint->Y=origPoint->Y;
        copiedPoint->Z=origPoint->Z;
        return copiedPoint;
    }

    void addTriangle(TriFloatXYZ *P1, TriFloatXYZ *P2, TriFloatXYZ *P3){
        Triangle *newFace = new Triangle;

        newFace->P1 = copyPoint(P1);
        newFace->P2 = copyPoint(P2);
        newFace->P3 = copyPoint(P3);

        normalUnitVector(newFace);
        faces.push_back(newFace);

    }

    void addFace(Triangle *newFace){
        addTriangle(newFace->P1,newFace->P2,newFace->P3);
    }

    void addSTL(STL_Binary* addShape){
        for(int i=0; i < addShape->numTriangles(); i++){
            addFace(addShape->getFace(i));
        }
    }

    int numTriangles(){
        return faces.size();
    }

    Triangle* getFace(int faceNum){
        return faces[faceNum];
    }

    void shiftSTL(float x, float y, float z){
        for(int i=0; i < faces.size(); i++){
            faces[i]->P1->X +=x;
            faces[i]->P2->X +=x;
            faces[i]->P3->X +=x;

            faces[i]->P1->Y +=y;
            faces[i]->P2->Y +=y;
            faces[i]->P3->Y +=y;

            faces[i]->P1->Z +=z;
            faces[i]->P2->Z +=z;
            faces[i]->P3->Z +=z;
        }
    }

    STL_Binary* copy(){
        STL_Binary* copied_stl = new STL_Binary;

        for(int i=0; i < faces.size(); i++){
            copied_stl->addFace(faces[i]); 
        }

        return copied_stl;
    }

    STL_Binary* shiftSTL_copy(float x, float y, float z){
        STL_Binary* copied_stl = copy();
        copied_stl->shiftSTL(x,y,z);
        return copied_stl;
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
        stl_stream.close();
        
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
    ~STL_Binary(){
        for(int i=0; i<faces.size(); i++){
            delete faces[i]->normal;
            delete faces[i]->P1;
            delete faces[i]->P2;
            delete faces[i]->P3;
            delete faces[i];
        }
        faces.clear();
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
    std::cout << "Creates 3D Model of Sphericon.";

    STL_Binary* SierpinskiPyramid = new STL_Binary;
    STL_Binary* SierpinskiPyramidCur = new STL_Binary;
    STL_Binary* SierpinskiPyramidPrev;

    float minPyrH = 1; //minimum pyramid height

    TriFloatXYZ tip = {minPyrH/2, minPyrH/2, minPyrH};
    TriFloatXYZ bottomTip = {minPyrH/2, minPyrH/2, -minPyrH};
    TriFloatXYZ p1 = {0,0,0};
    TriFloatXYZ p2 = {minPyrH,minPyrH,0};
    TriFloatXYZ p3 = {0,minPyrH,0};
    TriFloatXYZ p4 = {minPyrH,0,0};
    
    //top pyramid
    SierpinskiPyramid->addTriangle(&tip, &p1, &p4);
    SierpinskiPyramid->addTriangle(&tip, &p4, &p2);
    SierpinskiPyramid->addTriangle(&tip, &p2, &p3);
    SierpinskiPyramid->addTriangle(&tip, &p3, &p1);
    
    //bottom pyramid
    SierpinskiPyramid->addTriangle(&bottomTip, &p1, &p4);
    SierpinskiPyramid->addTriangle(&bottomTip, &p4, &p2);
    SierpinskiPyramid->addTriangle(&bottomTip, &p2, &p3);
    SierpinskiPyramid->addTriangle(&bottomTip, &p3, &p1);

    float shiftF = minPyrH; //shift factor
    for(int i=0; i<1; i++){
        //add bottom 0,0
        SierpinskiPyramidCur->addSTL(SierpinskiPyramid);
        
        //add bottom 0,1
        SierpinskiPyramidPrev = SierpinskiPyramid->shiftSTL_copy(0,shiftF,0); //shift base shape and copy
        SierpinskiPyramidCur->addSTL(SierpinskiPyramidPrev); //add shifted base shape to current shape
        delete SierpinskiPyramidPrev; //delete the shifted base shape
        
        //add bottom 1,1
        SierpinskiPyramidPrev = SierpinskiPyramid->shiftSTL_copy(shiftF,shiftF,0);
        SierpinskiPyramidCur->addSTL(SierpinskiPyramidPrev);
        delete SierpinskiPyramidPrev;
        
        //add bottom 1,0
        SierpinskiPyramidPrev = SierpinskiPyramid->shiftSTL_copy(shiftF,0,0);
        SierpinskiPyramidCur->addSTL(SierpinskiPyramidPrev);
        delete SierpinskiPyramidPrev;
        
        //add top
        SierpinskiPyramidPrev = SierpinskiPyramid->shiftSTL_copy(shiftF/2,shiftF/2,shiftF);
        SierpinskiPyramidCur->addSTL(SierpinskiPyramidPrev);
        delete SierpinskiPyramidPrev;

        //add bottom
        SierpinskiPyramidPrev = SierpinskiPyramid->shiftSTL_copy(shiftF/2,shiftF/2,-shiftF);
        SierpinskiPyramidCur->addSTL(SierpinskiPyramidPrev);
        delete SierpinskiPyramidPrev;
        
        //set up to repeat
        delete SierpinskiPyramid; //delete old base shape
        SierpinskiPyramid = SierpinskiPyramidCur->copy(); //assign base shape to be copy of new shape
        delete SierpinskiPyramidCur; //delete the old new shape
        SierpinskiPyramidCur = new STL_Binary; //set current shape to be empty object
        shiftF*=2; //adjust the shft factor
    }

    std::cout << "Generated with " << SierpinskiPyramid->numTriangles() << " faces." <<std::endl;

    SierpinskiPyramid->renderSTL("SierpinskiPyramid.stl");


    return 0;
}
