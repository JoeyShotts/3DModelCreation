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


void ParametricRotatedElipse(TriFloatXYZ* output, float t, float u, float a, float b, TriFloatXYZ* offset){
    float theta_1 = t/2; //pattern rotated about X (prior to translation)
    float theta_2 = t + M_PI/2; //pattern rotated about Z (prior to translation)
    // -sin(\theta_{2})(cos(\theta_{1})bcos(u)-sin(\theta_{1})asin(u)
    output->X = (-sin(theta_2)*(cos(theta_1)*b*cos(u) - sin(theta_1)*a*sin(u))) + offset->X;;
    // cos(\theta_{2})(cos(\theta_{1})bcos(u)-sin(\theta_{1})asin(u))
    output->Y =  (cos(theta_2)*(cos(theta_1)*b*cos(u) - sin(theta_1)*a*sin(u))) + offset->Y;
    // sin(\theta_{1})bcos(u)+asin(u)cos(\theta_{1})
    output->Z =  (sin(theta_1)*b*cos(u) + a*sin(u)*cos(theta_1)) + offset->Z;
}

int main(){
    std::cout << "Creates 3D Model of Sphericon.";

    STL_Binary mobiusStrip;

    TriFloatXYZ EllipsePath_Center = {80, 70, 30};

    float EllipsePath_a = 30;
    float EllipsePath_b = 50;
    float EllipsePath_h = 30; //height

    float pattern_a = 4;
    float pattern_b = 14;

    int numTriangles = 32000;

    int numPatternPoints = 100;
    int numPatterns = numTriangles/numPatternPoints/2;

    TriFloatXYZ EllipsePath_CurPoint;
    EllipsePath_CurPoint.Z = EllipsePath_h;
    TriFloatXYZ EllipsePath_PrevPoint = {(EllipsePath_Center.X+EllipsePath_b), EllipsePath_Center.Y, EllipsePath_h};

    //4 points are required, as 2 triangles are created one after the other
    TriFloatXYZ PrevPatternPathPrevPoint;
    TriFloatXYZ PrevPatternPathCurPoint;
    TriFloatXYZ CurPatternPathPrevPoint;
    TriFloatXYZ CurPatternPathCurPoint;


    float dt = 2*M_PI/numPatterns;
    float t = 0; //ellipse path theta iterating variable
    float prev_t = 0;

    float du = 2*M_PI/numPatternPoints;
    float u=0;
    
    //loop aorund outer mobius circle
    for(int i=1; i<=numPatterns;i++){
        t += dt;
        EllipsePath_CurPoint.X = EllipsePath_Center.X+EllipsePath_b*cos(t);
        EllipsePath_CurPoint.Y = EllipsePath_Center.Y+EllipsePath_a*sin(t);
        
        ParametricRotatedElipse(&PrevPatternPathPrevPoint,prev_t,0,pattern_a,pattern_b, &EllipsePath_PrevPoint); 
        ParametricRotatedElipse(&CurPatternPathPrevPoint, t, 0,pattern_a,pattern_b, &EllipsePath_CurPoint);

        //loop around inner mobius pattern
        u=0;
        for(int i=0; i<numPatternPoints; i++){
            u += du;

            ParametricRotatedElipse(&CurPatternPathCurPoint, t, u,pattern_a,pattern_b, &EllipsePath_CurPoint); 
            ParametricRotatedElipse(&PrevPatternPathCurPoint,prev_t, u,pattern_a,pattern_b, &EllipsePath_PrevPoint);
            
            //actually add the two triangles
            mobiusStrip.addTriangle(&CurPatternPathPrevPoint, &PrevPatternPathPrevPoint, &PrevPatternPathCurPoint);
            mobiusStrip.addTriangle(&PrevPatternPathCurPoint, &CurPatternPathCurPoint, &CurPatternPathPrevPoint);

            //set up for next iteration
            copyPoint(&PrevPatternPathPrevPoint, &PrevPatternPathCurPoint);
            copyPoint(&CurPatternPathPrevPoint, &CurPatternPathCurPoint);

        }

        prev_t = t;
        copyPoint(&EllipsePath_PrevPoint, &EllipsePath_CurPoint);


    }

   

    std::cout << "Generated with " << mobiusStrip.numTriangles() << " faces." <<std::endl;

    mobiusStrip.renderSTL("MobiusStrip.stl");


    return 0;
}
