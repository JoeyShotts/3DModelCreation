/*
Joseph Shotts
10/8/2025
C++ Code
Description: Creates a fractal tree. Code is in progress.
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

void copyPoint(TriFloatXYZ *copiedPoint, TriFloatXYZ *origPoint){
    copiedPoint->X=origPoint->X;
    copiedPoint->Y=origPoint->Y;
    copiedPoint->Z=origPoint->Z;
}

#define NUM_DIV 32

void horizCircle(float theta, float radius, TriFloatXYZ* offset, TriFloatXYZ* output){
    output->X=radius*cos(theta) + offset->X;
    output->Y=radius*sin(theta) + offset->Y;    
    output->Z=offset->Z;
}

void capBase(STL_Binary* base, TriFloatXYZ* offset, float radius){
    TriFloatXYZ PrevPatternPathPrevPoint;
    TriFloatXYZ PrevPatternPathCurPoint;

    float u;
    float du = 2*M_PI/NUM_DIV;

    horizCircle(0, radius, offset, &PrevPatternPathPrevPoint);

    //loop around circle, connecting two outer points to center
    u=0;
    for(int i=0; i<NUM_DIV; i++){
        u += du;

        horizCircle(u, radius, offset, &PrevPatternPathCurPoint);

        //actually add the two triangles
            base->addTriangle(offset, &PrevPatternPathCurPoint, &PrevPatternPathPrevPoint);

        //set up for next iteration
        copyPoint(&PrevPatternPathPrevPoint, &PrevPatternPathCurPoint);
    }
}

void trunkTree(STL_Binary* base, TriFloatXYZ* offset, float radius){
    TriFloatXYZ PrevPatternPathPrevPoint;
    TriFloatXYZ PrevPatternPathCurPoint;
    TriFloatXYZ CurPatternPathPrevPoint;
    TriFloatXYZ CurPatternPathCurPoint;
    TriFloatXYZ tempOffset;

    tempOffset.X = offset->X;
    tempOffset.Y = offset->Y;

    float z=0;
    float prev_z=0;
    float dz=radius*2/NUM_DIV;
    float u;
    float du = 2*M_PI/NUM_DIV;

    //loop from bottom to top of trunk
    for(int i=0; i<NUM_DIV;i++){
        z += dz;
        tempOffset.Z = prev_z;
        horizCircle(0, radius, &tempOffset, &CurPatternPathPrevPoint); 
        tempOffset.Z = z;
        horizCircle(0, radius, &tempOffset, &PrevPatternPathPrevPoint);

        //loop around trunk circle, connecting the previous circle to the current circle
        u=0;
        for(int i=0; i<NUM_DIV; i++){
            u += du;

            tempOffset.Z = prev_z;
            horizCircle(u, radius, &tempOffset, &CurPatternPathCurPoint); 
            tempOffset.Z = z;
            horizCircle(u, radius, &tempOffset, &PrevPatternPathCurPoint);

            //actually add the two triangles
            base->addTriangle(&CurPatternPathPrevPoint, &PrevPatternPathPrevPoint, &PrevPatternPathCurPoint);
            base->addTriangle(&PrevPatternPathCurPoint, &CurPatternPathCurPoint, &CurPatternPathPrevPoint);

            //set up for next iteration
            copyPoint(&PrevPatternPathPrevPoint, &PrevPatternPathCurPoint);
            copyPoint(&CurPatternPathPrevPoint, &CurPatternPathCurPoint);

        }
        prev_z = z;
    }
}

void baseTree(STL_Binary* base, TriFloatXYZ* offset, float radius, TriFloatXYZ* leftEndCapOff, TriFloatXYZ* rightEndCapOff){
    TriFloatXYZ PrevPatternPathPrevPoint;
    TriFloatXYZ PrevPatternPathCurPoint;
    TriFloatXYZ CurPatternPathPrevPoint;
    TriFloatXYZ CurPatternPathCurPoint;
    TriFloatXYZ tempOffset;

    tempOffset.X = offset->X;
    tempOffset.Y = offset->Y;

    //vertical iterator
    float z=radius*2;
    float prev_z=radius*2;
    float dz=radius/NUM_DIV;
    
    //circle angular iterator
    float u=0;
    float du = 2*M_PI/NUM_DIV;
    float start_u;
    float end_u;
    float prev_start_u = -M_PI_2;
    float prev_end_u = M_PI_2;

    //radius iterator
    float r = radius;
    float dr = radius/2/NUM_DIV;
    float prev_r = radius;

    //x offset iterator
    float x=0;
    float dx=radius/NUM_DIV;
    float prev_x=0;

    int start_i=1;
    int end_i=NUM_DIV;

    //loop from bottom to top, slowly reducing radius until it's halfed, slowly shifting circle center point
    for(int i=0; i<NUM_DIV;i++){
        z += dz;
        r -= dr;
        x += dx;

        //loop around circle
        //distance between circles = 2*x
        //full circle when 2*x > 2*r or x > r
        //for the right circle, start ang = (3Pi/2 - asin(x/r)) = -(2PI - (3Pi/2 - asin(x/r)) = -Pi/2-asin(x/r)
        //for the right circle, end ang = Pi/2 + asin(x/r)

        if(x > r){
            start_i=1;
            end_i=NUM_DIV;
            start_u=0;
            end_u=2*M_PI;
        }
        else{
            end_i = (int)((M_PI_2 + asin(x/r)) / du)+1;//round up
            end_u = (M_PI_2 + asin(x/r));
            start_i = (int)((-M_PI_2 - asin(x/r)) / du);//round down
            start_u = (-M_PI_2 - asin(x/r));
        }

        tempOffset.Z = z;
        tempOffset.X = offset->X+x;
        horizCircle(start_u, r, &tempOffset, &CurPatternPathPrevPoint); 
        tempOffset.Z = prev_z;
        tempOffset.X = offset->X+prev_x;
        horizCircle(prev_start_u, prev_r, &tempOffset, &PrevPatternPathPrevPoint);

        for(int i=start_i; i<=end_i; i++){
            u = i*du;
            if(i == end_i){
                tempOffset.Z = z;
                tempOffset.X = offset->X+x;
                horizCircle(end_u, r, &tempOffset, &CurPatternPathCurPoint); 
                tempOffset.Z = prev_z;
                tempOffset.X = offset->X+prev_x;
                horizCircle(prev_end_u, prev_r, &tempOffset, &PrevPatternPathCurPoint);
            }
            else{
                tempOffset.Z = z;
                tempOffset.X = offset->X+x;
                horizCircle(u, r, &tempOffset, &CurPatternPathCurPoint); 
                tempOffset.Z = prev_z;
                tempOffset.X = offset->X+prev_x;
                horizCircle(u, prev_r, &tempOffset, &PrevPatternPathCurPoint);
            }
            
            //actually add the two triangles 
            base->addTriangle(&CurPatternPathPrevPoint, &PrevPatternPathPrevPoint, &PrevPatternPathCurPoint);
            base->addTriangle(&PrevPatternPathCurPoint, &CurPatternPathCurPoint, &CurPatternPathPrevPoint);

            //set up for next iteration
            copyPoint(&PrevPatternPathPrevPoint, &PrevPatternPathCurPoint);
            copyPoint(&CurPatternPathPrevPoint, &CurPatternPathCurPoint);

        }
        prev_z = z;
        prev_r = r;
        prev_x = x;
        prev_start_u = start_u;
        prev_end_u = end_u;
    }

    // x=0;
    // prev_x=0;
    // r = radius;
    // prev_r = radius;
    // z=radius*2;
    // prev_z=radius*2;

    // start_i=1;
    // end_i=NUM_DIV;

    // //loop from bottom to top, slowly reducing radius until it's halfed, slowly shifting circle center point
    // for(int i=0; i<NUM_DIV;i++){
    //     z += dz;
    //     r -= dr;
    //     x -= dx;

    //     //loop around circle
    //     //distance between circles = 2*x
    //     //full circle when 2*x > 2*r or x > r
    //     //for the right circle, start ang = (3Pi/2 - asin(x/r)) = -(2PI - (3Pi/2 - asin(x/r)) = -Pi/2-asin(x/r)
    //     //for the right circle, end ang = Pi/2 + asin(x/r)

    //     if((-x) > r){
    //         start_i=-1;
    //         end_i=-NUM_DIV;
    //     }
    //     else{
    //         end_i = (int)((M_PI_2 + asin(x/r)) / du)-25;
    //         start_i = (int)((-M_PI_2 - asin(x/r)) / du);
    //     }

    //     tempOffset.Z = prev_z;
    //     tempOffset.X = offset->X+prev_x;
    //     horizCircle(0, prev_r, &tempOffset, &CurPatternPathPrevPoint); 
    //     tempOffset.Z = z;
    //     tempOffset.X = offset->X+x;
    //     horizCircle(0, r, &tempOffset, &PrevPatternPathPrevPoint);

    //     for(int i=start_i; i>=end_i; i--){
    //         u = i*du;

    //         tempOffset.Z = prev_z;
    //         tempOffset.X = offset->X+prev_x;
    //         horizCircle(u, prev_r, &tempOffset, &CurPatternPathCurPoint); 
    //         tempOffset.Z = z;
    //         tempOffset.X = offset->X+x;
    //         horizCircle(u, r, &tempOffset, &PrevPatternPathCurPoint);

    //         //actually add the two triangles
    //         base->addTriangle(&CurPatternPathPrevPoint, &PrevPatternPathPrevPoint, &PrevPatternPathCurPoint);
    //         base->addTriangle(&PrevPatternPathCurPoint, &CurPatternPathCurPoint, &CurPatternPathPrevPoint);

    //         //set up for next iteration
    //         copyPoint(&PrevPatternPathPrevPoint, &PrevPatternPathCurPoint);
    //         copyPoint(&CurPatternPathPrevPoint, &CurPatternPathCurPoint);

    //     }
    //     prev_z = z;
    //     prev_r = r;
    //     prev_x = x;
    // }
}

void baseTreeR(STL_Binary* base){

}

int main(){
    std::cout << "Creates 3D Model of Fractal Tree.";

    STL_Binary FractalTree;
    TriFloatXYZ baseOffset;
    TriFloatXYZ leftOffTemp;
    TriFloatXYZ rightOffTemp; 

    float baseRadius = 2;
    // trunkTree(&FractalTree, &baseOffset, baseRadius);
    baseTree(&FractalTree, &baseOffset, baseRadius, &leftOffTemp, &rightOffTemp);
    // capBase(&FractalTree, &baseOffset, baseRadius);
    std::cout << "Generated with " << FractalTree.numTriangles() << " faces." <<std::endl;

    FractalTree.renderSTL("FractalTree.stl");

    return 0;
}
