#include "body.h"

#include <iostream>

Body::Body()
{
    ImportModel("../resources/sphere.x3d");
}

Body::Body(Body&& body)
{
    using namespace std;
    mass = body.mass;
    J = move(body.J);
    scale = move(body.scale);
    postition = move(body.postition);
    velocity = move(body.velocity);
    acceleration = move(body.acceleration);
    orientation = move(body.orientation);
    angularVelocity = move(body.angularVelocity);
    angularAcceleration = move(body.angularAcceleration);
}

Body::Body(const Body& body)
{
    std::scoped_lock guard(body.mtx);
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

void Body::draw(QOpenGLShaderProgram& program, Camera& cam)
{

    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program.attributeLocation("a_position");
    vertexLocation = 0;
    program.enableAttributeArray(vertexLocation);
    program.setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, vertices.size());

    // Offset for texture coordinate
    // offset += sizeof(QVector3D);

    // // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    // int texcoordLocation = program.attributeLocation("a_texcoord");
    // program.enableAttributeArray(texcoordLocation);
    // program.setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_TRIANGLE_STRIP, numberOfFaces, GL_UNSIGNED_SHORT, nullptr);
}

void Body::update()
{
    std::scoped_lock guard(mtx);
}

Eigen::Matrix4f Body::getBodyMatrix() const
{
    std::scoped_lock guard(mtx);
    Eigen::Affine3f bodyMatrix;
    bodyMatrix.setIdentity();
    bodyMatrix.scale(scale);
    bodyMatrix.rotate(orientation);
    bodyMatrix.translate(postition);

    return bodyMatrix.matrix();
}

bool Body::ImportModel(std::string pFile)
{
    Assimp::Importer importer;

    aiScene* scene_ = const_cast<aiScene*>(importer.ReadFile( pFile,
                                aiProcess_CalcTangentSpace       |
                                aiProcess_Triangulate            |
                                aiProcess_JoinIdenticalVertices  |
                                aiProcess_SortByPType));

    if( !scene_)
    {
        std::cerr << importer.GetErrorString() << "\n";
        return false;
    }

    for(uint32_t i = 0; i < scene_->mMeshes[0]->mNumVertices;i++)
    {
        vertices.push_back( Eigen::Vector3f(
            scene_->mMeshes[0]->mVertices[i].x,
            scene_->mMeshes[0]->mVertices[i].y,
            scene_->mMeshes[0]->mVertices[i].z
        ));
    }

    numberOfFaces = scene_->mMeshes[0]->mNumFaces;

    for(uint32_t i = 0; i < scene_->mMeshes[0]->mNumFaces;i++)
    {
        for(uint32_t j = 0; j < scene_->mMeshes[0]->mFaces[i].mNumIndices; j++)
        {
            indices.push_back(scene_->mMeshes[0]->mFaces[i].mIndices[j]);
        }
    }

    arrayBuf.bind();
    arrayBuf.allocate(vertices.data(), vertices.size() * sizeof(vertices[0]));

    indexBuf.bind();
    indexBuf.allocate(indices.data(), indices.size() * sizeof(indices[0]));

    return true;
}
