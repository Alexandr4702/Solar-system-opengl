#include "body.h"

#include <iostream>

Body::Body()
{
    ImportModel("../resources/sphere.x3d");
}

Body::Body(Body&& body)
{
    scene = body.scene;
    mass = body.mass;
    J = body.J;
    scale = body.scale;
    postition = body.postition;
    velocity = body.velocity;
    acceleration = body.acceleration;
    orientation = body.orientation;
    angularVelocity = body.angularVelocity;
    angularAcceleration = body.angularAcceleration;
}

Body::Body(const Body& body)
{
    std::scoped_lock guard(body.mtx);
    scene = body.scene;
    mass = body.mass;
    J = body.J;
    scale = body.scale;
    postition = body.postition;
    velocity = body.velocity;
    acceleration = body.acceleration;
    orientation = body.orientation;
    angularVelocity = body.angularVelocity;
    angularAcceleration = body.angularAcceleration;
}

Body::~Body()
{
}

void Body::draw()
{

}

void Body::update()
{
    std::scoped_lock guard(mtx);
}

Eigen::Matrix4f Body::getBodyMatrix() const
{
    std::scoped_lock guard(mtx);
    Eigen::Affine3f CamMatrix;
    CamMatrix.setIdentity();
    CamMatrix.scale(scale);
    CamMatrix.rotate(orientation);
    CamMatrix.translate(postition);

    return CamMatrix.matrix();
}

bool Body::ImportModel(std::string pFile)
{
    // Create an instance of the Importer class
    Assimp::Importer importer;
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // propably to request more postprocessing than we do in this example.
    scene = importer.ReadFile( pFile,
                                aiProcess_CalcTangentSpace       |
                                aiProcess_Triangulate            |
                                aiProcess_JoinIdenticalVertices  |
                                aiProcess_SortByPType);

    // If the import failed, report it
    if( !scene)
    {
    std::cerr << importer.GetErrorString() << "\n";
    return false;
    }
    // Now we can access the file's contents.
    //   DoTheSceneProcessing( scene);
    // We're done. Everything will be cleaned up by the importer destructor
    return true;
}
