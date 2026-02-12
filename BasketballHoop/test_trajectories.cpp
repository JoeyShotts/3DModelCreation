#include <thread>
#include <fstream> 
#include <iostream>
#include <cmath>
#include <vector>
#include <string.h>
#include <chrono>

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

void findTrajectory(TriFloatXYZ* ballEnd, TriFloatXYZ* bouncePoint, TriFloatXYZ* ballStart, TriFloatXYZ* faceNV, float maxH);
void testTrajectories(Triangle* face, float maxH, TriFloatXYZ* ballEnd, TriFloatXYZ* ballStart);

int main(){
    float maxH = 220; //200-245
    TriFloatXYZ ballStart = {20, 20, 0}; //relative to front corner of shooter box
    TriFloatXYZ ballEnd;
    TriFloatXYZ N = {0,0,0}; //normal (ignored)
    TriFloatXYZ P1 = {17,3,8.5};
    TriFloatXYZ P2 = {17,3.034,10.2};
    TriFloatXYZ P3 = {18.7,3.068,10.2};

    Triangle middleTest = {(&N), &P1, &P2, &P3};
    testTrajectories(&middleTest, maxH, &ballEnd, &ballStart);
    std::cout << "Result:";
    std::cout << " X: "<<ballEnd.X;
    std::cout << " Y: "<<ballEnd.Y;
    std::cout << " Z: "<<ballEnd.Z;

    // Triangle bottomLeft = {&(TriFloatXYZ){0,0,0}, &(TriFloatXYZ){0,7.25,0}, &(TriFloatXYZ){0,6.944,1.7}, &(TriFloatXYZ){1.7,6.298,1.7}};

    // Triangle topLeft = {&(TriFloatXYZ){0,0,0}, &(TriFloatXYZ){0,6.944,15.3}, &(TriFloatXYZ){0,7.25,17}, &(TriFloatXYZ){1.7,6.604,17}};

    // Triangle flat = {&(TriFloatXYZ){0,0,0}, &(TriFloatXYZ){0,6.944,17}, &(TriFloatXYZ){0,7.25,17}, &(TriFloatXYZ){1.7,6.604,17}};

    return 0;
}

//based on typical height of an individual, shooting just over the head
const float shootHeightMin = 155; //5ft
// const float shootHeightMax = 200; //6.5 ft
// const float maxArcHeight = 245; //around 8ft or the typical ceiling height

const float ballRadius = 5.08;
const float targetY = 15-ballRadius;
const TriFloatXYZ targetPoint = {11.5f, targetY, -1.5f};

const float minShootDistance = 50; //min distance from target, 1.6 ft
const float shootBoxWidth = 40; //shooter box width
const float shootBoxDepth = 40; //shooter box depth
const float shootBoxXStart = targetPoint.X - shootBoxWidth/2;

void testTrajectories(Triangle* face, float maxH, TriFloatXYZ* ballEnd, TriFloatXYZ* ballStart){
    ballEnd->Z = targetPoint.Z; //always the same
    TriFloatXYZ bouncePoint;
    TriFloatXYZ faceNV; //normal unit vector to face

    getNormal(face);

    ballStart->X += shootBoxXStart;
    ballStart->Y += minShootDistance;
    ballStart->Z += shootHeightMin;

    //get center point of face
    bouncePoint.X = (face->P1->X + face->P2->X + face->P3->X)/3; 
    bouncePoint.Y = (face->P1->Y + face->P2->Y + face->P3->Y)/3; 
    bouncePoint.Z = (face->P1->Z + face->P2->Z + face->P3->Z)/3; 

    findTrajectory(ballEnd, &bouncePoint, ballStart, &faceNV, maxH);

}

//FINDING TRAJECTORY **************************************************************************************************************
//created externally to avoid creating a bunch of times
TriFloatXYZ v2; //vector before bounce
TriFloatXYZ v3; //vector after bounce

const float g_a = 980.665; //cm/s^2
const float ballBounceRestitution = 0.5; //how bouncy the ball is

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
