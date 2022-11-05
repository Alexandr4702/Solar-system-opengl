#include "body.h"

#include <iostream>

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

    ImportModel("../resources/Sphere.stl");
    // ImportTestModel();
}

Body::Body(QOpenGLContext* context, std::string filename): QOpenGLFunctions(context), ctx(context),
indexBuf( new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer )),
arrayBuf( new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer))
{
    arrayBuf->create();
    indexBuf->create();

    ImportModel(filename);
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

void Body::draw(QOpenGLShaderProgram& program)
{
    arrayBuf->bind();
    indexBuf->bind();

    int vertexLocation = program.attributeLocation("a_position");
    vertexLocation = 0;
    program.enableAttributeArray(vertexLocation);
    program.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(vertices[0]));

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
}

void Body::draw(QOpenGLShaderProgram& program, Eigen::Matrix4f& matrixCam)
{
    // mvp[0][0];

    Eigen::Matrix4f mvpMat = (matrixCam * getBodyMatrix()).transpose();

    QMatrix4x4 matrix(mvpMat.data());
    program.setUniformValue("mvp_matrix", matrix);

    draw(program);
}

void Body::update()
{
    std::scoped_lock guard(mtx);
}

void Body::setBodyPosition(Eigen::Vector3f& pos)
{
    std::scoped_lock guard(mtx);
    postition = pos;
}

void Body::translateBody(Eigen::Vector3f& translation)
{
    std::scoped_lock guard(mtx);
    postition += translation;
}

void Body::setBodyRotation(Eigen::Quaternionf& q)
{
    std::scoped_lock guard(mtx);
    orientation = q;
}

void Body::rotateBody(Eigen::Quaternionf& q)
{
    std::scoped_lock guard(mtx);
    orientation = q * orientation;
}

Eigen::Matrix4f Body::getBodyMatrix() const
{
    std::scoped_lock guard(mtx);
    Eigen::Affine3f bodyMatrix;
    bodyMatrix.setIdentity();
    bodyMatrix.translate(postition);
    bodyMatrix.rotate(orientation);
    bodyMatrix.scale(scale);

    return bodyMatrix.matrix();
}

bool Body::ImportModel(std::string pFile)
{
    Assimp::Importer importer;

    const aiScene* scene_ = importer.ReadFile( pFile,
                                aiProcess_CalcTangentSpace       |
                                aiProcess_Triangulate            |
                                aiProcess_JoinIdenticalVertices  |
                                aiProcess_SortByPType
                                );

    if( !scene_)
    {
        std::cerr << importer.GetErrorString() << "\n";
        return false;
    }

    for(uint32_t i = 0; i < scene_->mMeshes[0]->mNumVertices;i++)
    {
        Eigen::Vector3f vec = Eigen::Vector3f(
            scene_->mMeshes[0]->mVertices[i].x,
            scene_->mMeshes[0]->mVertices[i].y,
            scene_->mMeshes[0]->mVertices[i].z
        );
        vertices.push_back( vec);
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
    std::vector <VertexData> cubeVertices = {
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(0.0f, 0.0f)},
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D(0.33f, 0.0f)},
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(0.0f, 0.5f)},
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.33f, 0.5f)},

        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D( 0.0f, 0.5f)},
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(0.33f, 0.5f)},
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.0f, 1.0f)},
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.33f, 1.0f)},

        {QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(0.66f, 0.5f)},
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(1.0f, 0.5f)},
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.66f, 1.0f)},
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(1.0f, 1.0f)},

        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(0.66f, 0.0f)},
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(1.0f, 0.0f)},
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(0.66f, 0.5f)},
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(1.0f, 0.5f)},

        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(0.33f, 0.0f)},
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(0.66f, 0.0f)},
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(0.33f, 0.5f)},
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D(0.66f, 0.5f)},

        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(0.33f, 0.5f)},
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.66f, 0.5f)},
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(0.33f, 1.0f)},
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.66f, 1.0f)}
    };

    std::vector<GLushort> cubeIndices = {
         0,  1,  2,  3,  3,
         4,  4,  5,  6,  7,  7,
         8,  8,  9, 10, 11, 11,
        12, 12, 13, 14, 15, 15,
        16, 16, 17, 18, 19, 19,
        20, 20, 21, 22, 23
    };

    for (auto&& vertex: cubeVertices)
    {
        vertices.push_back({vertex.position.x(), vertex.position.y(), vertex.position.z()});
    }

    for (auto&& index: cubeIndices)
    {
        indices.push_back(index);
    }

    arrayBuf->bind();
    arrayBuf->allocate(vertices.data(), vertices.size() * sizeof(vertices[0]));

    indexBuf->bind();
    indexBuf->allocate(indices.data(), indices.size() * sizeof(indices[0]));
    return true;
}
