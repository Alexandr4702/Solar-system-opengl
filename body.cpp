#include "body.h"

#include <iostream>

void drawCube(QOpenGLFunctions* f)
{
    double vertex[] =
    {
        -1.f,  1.f, -1.f,   255.0,
        1.f,  1.f, -1.f,    255.0,
        -1.f, -1.f, -1.f,   255.0,
        1.f, -1.f, -1.f,    255.0,
        -1.f, -1.f,  1.f,   255.0,
        -1.f,  1.f,  1.f,   255.0,
        1.f,  1.f,  1.f,    255.0,
        1.f, -1.f,  1.f,    255.0,
        -1.f, -1.f, -1.f,   255.0,
        -1.f,  1.f, -1.f,   255.0,
        -1.f,  1.f,  1.f,   255.0,
        -1.f, -1.f,  1.f,   255.0,
        1.f, -1.f, -1.f,    255.0,
        1.f,  1.f, -1.f,    255.0,
        1.f,  1.f,  1.f,    255.0,
        1.f, -1.f,  1.f,    255.0,
        -1.f, -1.f,  1.f,   255.0,
        -1.f, -1.f, -1.f,   255.0,
        1.f, -1.f, -1.f,    255.0,
        1.f, -1.f,  1.f,    255.0,
        -1.f,  1.f,  1.f,   255.0,
        -1.f,  1.f, -1.f,   255.0,
        1.f,  1.f, -1.f,    255.0,
        1.f,  1.f,  1.f    ,255.0,
    };

    f->glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(vertex[0]) * 4, vertex);
    f->glEnableVertexAttribArray(0);

    // f->glVertexAttribPointer(1, 1, GL_DOUBLE, GL_FALSE, sizeof(vertex[0]) * 4, vertex+3);
    // f->glEnableVertexAttribArray(1);

    f->glDrawArrays(GL_QUADS, 0, 24);
}

void Body::drawCube()
{
    double vertex[] =
    {
        -1.f,  1.f, -1.f,   255.0,
        1.f,  1.f, -1.f,    255.0,
        -1.f, -1.f, -1.f,   255.0,
        1.f, -1.f, -1.f,    255.0,
        -1.f, -1.f,  1.f,   255.0,
        -1.f,  1.f,  1.f,   255.0,
        1.f,  1.f,  1.f,    255.0,
        1.f, -1.f,  1.f,    255.0,
        -1.f, -1.f, -1.f,   255.0,
        -1.f,  1.f, -1.f,   255.0,
        -1.f,  1.f,  1.f,   255.0,
        -1.f, -1.f,  1.f,   255.0,
        1.f, -1.f, -1.f,    255.0,
        1.f,  1.f, -1.f,    255.0,
        1.f,  1.f,  1.f,    255.0,
        1.f, -1.f,  1.f,    255.0,
        -1.f, -1.f,  1.f,   255.0,
        -1.f, -1.f, -1.f,   255.0,
        1.f, -1.f, -1.f,    255.0,
        1.f, -1.f,  1.f,    255.0,
        -1.f,  1.f,  1.f,   255.0,
        -1.f,  1.f, -1.f,   255.0,
        1.f,  1.f, -1.f,    255.0,
        1.f,  1.f,  1.f    ,255.0,
    };

    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(vertex[0]) * 4, vertex);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_QUADS, 0, 24);

    // f->glVertexAttribPointer(1, 1, GL_DOUBLE, GL_FALSE, sizeof(vertex[0]) * 4, vertex+3);
    // f->glEnableVertexAttribArray(1);
}

Body::Body(QOpenGLContext* context): QOpenGLFunctions(context), ctx(context),
indexBuf( new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer )),
arrayBuf( new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer))
{
    arrayBuf->create();
    indexBuf->create();

    // ImportModel("../resources/sphere.x3d");
    ImportTestModel();
}

Body::Body(Body&& body): QOpenGLFunctions(body.ctx)
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

    numberOfFaces = body.numberOfFaces;
    vertices = move(body.vertices);
    indices  = move(body.indices);
    arrayBuf = move(body.arrayBuf);
    indexBuf = move(body.indexBuf);

    ctx = body.ctx;
    body.ctx = nullptr;
}

Body::~Body()
{
    if(arrayBuf.get() != nullptr)
    {
            arrayBuf->destroy();
    }
    if(indexBuf.get() != nullptr)
    {
            indexBuf->destroy();
    }
}

void Body::draw(QOpenGLShaderProgram& program, Camera& cam)
{
    // Tell OpenGL which VBOs to use
    // arrayBuf.bind();
    // indexBuf.bind();

    // // Offset for position
    // quintptr offset = 0;

    // // Tell OpenGL programmable pipeline how to locate vertex position data
    // int vertexLocation = program.attributeLocation("a_position");
    // vertexLocation = 0;
    // program.enableAttributeArray(vertexLocation);
    // program.setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(vertices[0]));

    // Offset for texture coordinate
    // offset += sizeof(QVector3D);

    // // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    // int texcoordLocation = program.attributeLocation("a_texcoord");
    // program.enableAttributeArray(texcoordLocation);
    // program.setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    // Draw cube geometry using indices from VBO 1
    // glDrawElements(GL_TRIANGLE_STRIP, numberOfFaces, GL_UNSIGNED_SHORT, nullptr);

    // f->glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(vertices[0]) * 4, vertices.data());
    // f->glEnableVertexAttribArray(0);
    // drawCube(f);
    // drawCube();


    // Tell OpenGL which VBOs to use
    arrayBuf->bind();
    indexBuf->bind();

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program.attributeLocation("a_position");
    program.enableAttributeArray(vertexLocation);
    program.setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int texcoordLocation = program.attributeLocation("a_texcoord");
    program.enableAttributeArray(texcoordLocation);
    program.setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_TRIANGLE_STRIP, 34, GL_UNSIGNED_SHORT, nullptr);
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

    arrayBuf->bind();
    arrayBuf->allocate(vertices.data(), vertices.size() * sizeof(vertices[0]));

    indexBuf->bind();
    indexBuf->allocate(indices.data(), indices.size() * sizeof(indices[0]));

    return true;
}

bool Body::ImportTestModel()
{
    // For cube we would need only 8 vertices but we have to
    // duplicate vertex for each face because texture coordinate
    // is different.
    VertexData vertices[] = {
        // Vertex data for face 0
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(0.0f, 0.0f)},  // v0
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D(0.33f, 0.0f)}, // v1
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(0.0f, 0.5f)},  // v2
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.33f, 0.5f)}, // v3

        // Vertex data for face 1
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D( 0.0f, 0.5f)}, // v4
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(0.33f, 0.5f)}, // v5
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.0f, 1.0f)},  // v6
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.33f, 1.0f)}, // v7

        // Vertex data for face 2
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(0.66f, 0.5f)}, // v8
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(1.0f, 0.5f)},  // v9
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.66f, 1.0f)}, // v10
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(1.0f, 1.0f)},  // v11

        // Vertex data for face 3
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(0.66f, 0.0f)}, // v12
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(1.0f, 0.0f)},  // v13
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(0.66f, 0.5f)}, // v14
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(1.0f, 0.5f)},  // v15

        // Vertex data for face 4
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(0.33f, 0.0f)}, // v16
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(0.66f, 0.0f)}, // v17
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(0.33f, 0.5f)}, // v18
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D(0.66f, 0.5f)}, // v19

        // Vertex data for face 5
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(0.33f, 0.5f)}, // v20
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.66f, 0.5f)}, // v21
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(0.33f, 1.0f)}, // v22
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.66f, 1.0f)}  // v23
    };

    // Indices for drawing cube faces using triangle strips.
    // Triangle strips can be connected by duplicating indices
    // between the strips. If connecting strips have opposite
    // vertex order then last index of the first strip and first
    // index of the second strip needs to be duplicated. If
    // connecting strips have same vertex order then only last
    // index of the first strip needs to be duplicated.
    GLushort indices[] = {
         0,  1,  2,  3,  3,     // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
         4,  4,  5,  6,  7,  7, // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
         8,  8,  9, 10, 11, 11, // Face 2 - triangle strip ( v8,  v9, v10, v11)
        12, 12, 13, 14, 15, 15, // Face 3 - triangle strip (v12, v13, v14, v15)
        16, 16, 17, 18, 19, 19, // Face 4 - triangle strip (v16, v17, v18, v19)
        20, 20, 21, 22, 23      // Face 5 - triangle strip (v20, v21, v22, v23)
    };

//! [1]
    // Transfer vertex data to VBO 0
    arrayBuf->bind();
    arrayBuf->allocate(vertices, 24 * sizeof(VertexData));

    // Transfer index data to VBO 1
    indexBuf->bind();
    indexBuf->allocate(indices, 34 * sizeof(GLushort));
    return true;
}
