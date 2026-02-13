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

//used when testing individual faces in seperate threads
struct faceTest{
    TriFloatXYZ P1;
    TriFloatXYZ P2;
    TriFloatXYZ P3;
    TriFloatXYZ P4;
    float performance;
    int64_t time;
    bool  testCompleted;
};

void testFace(void* faceIn);
void findTrajectory(TriFloatXYZ* ballEnd, TriFloatXYZ* bouncePoint, TriFloatXYZ* ballStart, TriFloatXYZ* faceNV, float maxH);

//STL Class Definitions **************************************************************
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

int main(){
    std::cout << "Testing Three Faces: \n";
    faceTest middleTest = {{17,3,8.5}, {17,3.034,10.2}, {18.7,3.068,10.2}, {18.7,3.034, 8.5}, 0,0,false};
    testFace(&middleTest);
    std::cout << "Middle Test(0,0): " << middleTest.performance<<"\n";

    faceTest bottomLeft = {{0,7.25,0}, {0,6.944,1.7}, {1.7,6.298,1.7}, {1.7,6.604, 0}, 0,0,false};
    testFace(&bottomLeft);
    std::cout << "Bottom Left Test(-halfHoriz,-halfVert,0): " << bottomLeft.performance<<"\n";

    faceTest topLeft = {{0,6.944,15.3}, {0,7.25,17}, {1.7,6.604,17}, {1.7,6.298, 15.3}, 0,0,false};
    testFace(&topLeft);
    std::cout << "Top Left Test(-halfHoriz, halfVert,0): " << topLeft.performance<<"\n";

    //I'm not sure about this test, I think it's more succefful than it should have been
    faceTest flat = {{0,6.944,17}, {0,7.25,17}, {1.7,6.604,17}, {1.7,6.298, 17}, 0,0,false};
    testFace(&flat);
    std::cout << "Flat: " << topLeft.performance<<"\n";

    return 0;
}

//Constants for Single Face Test*********************************************
const float ballRadius = 5.08f;
const float targetY = 11.5f-ballRadius;
const TriFloatXYZ targetPoint = {16.0f, targetY, -1.5f};
const float originToGnd = 186.2f; //origin height from ground

const int maxNumExceptions = 10; //max num exceptions that occur when finding trajectory
//shooter area: all area that a shot may occur from
//target: target point for a ball to hit 

const float maxTargetDis = 4; //max distance from target that will likely still go in the hoop
const float maxTargetDisSquared = maxTargetDis*maxTargetDis;

const float dShooter = 1; //defines the iterating size in cm over the shooter area

//based on typical height of an individual, shooting just over the head
const float shootHeightMin = 155.0f; //5ft
const float shootHeightMax = 200.0f; //6.5 ft

const float maxArcHeight = 245; //around 8ft or the typical ceiling height

// minAcceptablemaxHOffset means that the parabolic arc of a target must at least 
// have a curve that deviates by 5cm vertically. 
// This also bounds the max speed the ball can be thrown.
const float minAcceptablemaxHOffset = 10; 

const float minShootDistance = 50; //min distance from target, 1.6 ft
const float shootBoxWidth = 100; //shooter box width
const float shootBoxDepth = 100; //shooter box depth
const float shootBoxHeight = shootHeightMax-shootHeightMin; 
const float shootBoxXStart = targetPoint.X - shootBoxWidth/2;

const int shootBoxXDiv = (int)(shootBoxWidth/dShooter);
const int shootBoxYDiv = (int)(shootBoxDepth/dShooter);
const int shootBoxZDiv = (int)(shootBoxHeight/dShooter);
const int maxHDiv      = (int)((maxArcHeight-shootHeightMin)/dShooter);
const double numShootPos  = (double)shootBoxXDiv*(double)shootBoxYDiv*(double)shootBoxZDiv*(double)maxHDiv; //this is an upper bound, not a true value

//tests a single face defined in the faceTest structure
//designed so that it only access faceTest structure and can consequentially run in a seperate thread
void testFace(void* faceIn){
    faceTest* face = (faceTest*)faceIn;

    auto start = std::chrono::high_resolution_clock::now();
    int numExceptions = 0;
    int numTargetHits = 0;
    TriFloatXYZ ballEnd;
    ballEnd.Z = targetPoint.Z; //always the same
    TriFloatXYZ bouncePoint;
    TriFloatXYZ ballStart;
    TriFloatXYZ faceNV; //normal unit vector to face

    float maxH=0;
    float minAcceptableMaxHeight;
    float ballDis;

    Triangle faceNormal = {&faceNV, &(face->P1), &(face->P2), &(face->P3)};
    getNormal(&faceNormal);

    //get center point of face
    bouncePoint.X = (face->P1.X + face->P2.X + face->P3.X + face->P4.X)/4; 
    bouncePoint.Y = (face->P1.Y + face->P2.Y + face->P3.Y + face->P4.Y)/4; 
    bouncePoint.Z = (face->P1.Z + face->P2.Z + face->P3.Z + face->P4.Z)/4; 

    //iterate through all shooting positions
    for(int i=(-shootBoxXDiv/2); i<(shootBoxXDiv/2); i++){
        for(int j=(-shootBoxYDiv/2); j<(shootBoxYDiv/2); j++){
            for(int k=0; k<shootBoxZDiv; k++){
                //determine ball starting point
                ballStart.X = i*dShooter + shootBoxXStart;
                ballStart.Y = j*dShooter + minShootDistance;
                ballStart.Z = k*dShooter + shootHeightMin - originToGnd; //relative to origin

                //ensures that the maxH the ball reaches still follows a parabolic arc
                minAcceptableMaxHeight = (bouncePoint.Z-ballStart.Z)/2 + ballStart.Z + minAcceptablemaxHOffset; //relative to ground

                for(int l=0; l<maxHDiv; l++){
                    maxH = l*dShooter + shootHeightMin; //relative to ground
                    if(maxH < minAcceptableMaxHeight){
                        continue;
                    }
                    maxH -= originToGnd; // make it relative to origin

                    //try to find a target. As this happens a lot (and math erros could happen), a simple try-except block was added.
                    try {
                        findTrajectory(&ballEnd, &bouncePoint, &ballStart, &faceNV, maxH);
                    }
                    catch(...){
                        numExceptions++;
                        std::cout << "Exception occured when finding trajectory. " << numExceptions << " have occured.";
                        if(numExceptions > maxNumExceptions){
                            std::cout << "Max Exceptions occured. Program exiting.";
                            exit(1);
                        }
                        continue; //don't test against the target
                    }
                    //test if ballEnd is within exceptable range of target
                    ballDis = fabs(ballEnd.X-targetPoint.X) + fabs(ballEnd.Y-targetPoint.Y);

                    //if target was hit
                    if(ballDis < maxTargetDisSquared){
                        numTargetHits++;
                    }
                }
            }
        }
    }

    face->performance = ((float)numTargetHits)/numShootPos; //percentage of successful shots

    auto stop = std::chrono::high_resolution_clock::now();
    
    //calculate the duration it took
    face->time = (int)(std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)).count();
    face->testCompleted = true;
}

//FINDING TRAJECTORY **************************************************************************************************************
//created externally to avoid creating a bunch of times
TriFloatXYZ v2; //vector before bounce
TriFloatXYZ v3; //vector after bounce

//defines parameters for target relative to origin
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