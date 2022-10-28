#include "body.h"

#include <iostream>

Body::Body()
{
    ImportModel("../sphere.x3d");
}

void Body::draw()
{

}

void Body::update()
{

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
