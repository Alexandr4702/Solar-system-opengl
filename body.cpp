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
    texture = move(body.texture);

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

    // int vertexLocation = program.attributeLocation("a_position");
    int vertexLocation = 0;
    program.enableAttributeArray(vertexLocation);
    program.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(vertices[0]));

    int texture_coordinate_loaction = 1;
    program.enableAttributeArray(texture_coordinate_loaction);
    program.setAttributeBuffer(texture_coordinate_loaction, GL_FLOAT, offsetof(VertexData, texCoord), 2, sizeof(vertices[0]));

    int normal_location = 2;
    program.enableAttributeArray(normal_location);
    program.setAttributeBuffer(normal_location, GL_FLOAT, offsetof(VertexData, normal), 2, sizeof(vertices[0]));

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
}

void Body::draw(QOpenGLShaderProgram& program, Eigen::Matrix4f& matrixCam)
{
    // mvp[0][0];
    if(texture.get() != nullptr)
        texture->bind();

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

void Body::setBodyScale(Eigen::Vector3f& scale)
{
    std::scoped_lock guard(mtx);
    this->scale = scale;
}

void Body::setBodyScale(Eigen::Vector3f &&scale)
{
    std::scoped_lock guard(mtx);
    this->scale = scale;
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
    aiString texture_path;
    unsigned int texture_index;
    QImage texture_image;
    std::filesystem::path path_to_file(pFile);

    const aiScene* scene_ = importer.ReadFile( pFile,
                                aiProcess_Triangulate            |
                                aiProcess_GenSmoothNormals       |
                                aiProcess_OptimizeMeshes         |
                                aiProcess_JoinIdenticalVertices  |
                                aiProcess_OptimizeGraph          |
                                aiProcess_SortByPType
                                );

    if( !scene_)
    {
        std::cerr << importer.GetErrorString() << "\n";
        return false;
    }

    for(uint32_t i = 0; i < scene_->mNumMaterials; i++)
    {
        std::cout << (scene_->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE)) << " " << 1 << " " << i << " \n";
        if((scene_->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE)) > 0)
        {
            scene_->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path, NULL, &texture_index);
        }
    }

    std::cout << path_to_file.parent_path().string() << "\n";
    QString path_to_texture = QString(path_to_file.parent_path().string().c_str()) + QString("/") + QString(texture_path.C_Str());
    qDebug() << path_to_texture;
    texture_image = QImage(path_to_texture);

    if (!texture_image.isNull())
    {
        texture = std::shared_ptr<QOpenGLTexture> (new QOpenGLTexture(texture_image.mirrored()));
    }

    for(uint32_t i = 0; i < scene_->mMeshes[0]->mNumVertices;i++)
    {
        VertexData vertex;
        vertex.position = Eigen::Vector3f(
        scene_->mMeshes[0]->mVertices[i].x,
        scene_->mMeshes[0]->mVertices[i].y,
        scene_->mMeshes[0]->mVertices[i].z
        );

        if(scene_->mMeshes[0]->mTextureCoords[texture_index] != nullptr)
        {
            vertex.texCoord = Eigen::Vector2f(
                scene_->mMeshes[0]->mTextureCoords[texture_index][i].x,
                scene_->mMeshes[0]->mTextureCoords[texture_index][i].y
            );
        }

        vertex.normal = Eigen::Vector3f(
        scene_->mMeshes[0]->mNormals[i].x,
        scene_->mMeshes[0]->mNormals[i].y,
        scene_->mMeshes[0]->mNormals[i].z
        );
        vertices.push_back(vertex);
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
        { {-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f } },
        { { 1.0f, -1.0f,  1.0f}, {0.33f, 0.0f} },
        { {-1.0f,  1.0f,  1.0f}, {0.0f, 0.5f } },
        { { 1.0f,  1.0f,  1.0f}, {0.33f, 0.5f} },

        { {1.0f, -1.0f,  1.0f}, { 0.0f, 0.5f} },
        { {1.0f, -1.0f, -1.0f}, {0.33f, 0.5f} },
        { {1.0f,  1.0f,  1.0f}, {0.0f , 1.0f} },
        { {1.0f,  1.0f, -1.0f}, {0.33f, 1.0f} },

        { { 1.0f, -1.0f, -1.0f}, { 0.66f, 0.5f}},
        { {-1.0f, -1.0f, -1.0f}, { 1.0f , 0.5f}},
        { { 1.0f,  1.0f, -1.0f}, { 0.66f, 1.0f}},
        { {-1.0f,  1.0f, -1.0f}, { 1.0f , 1.0f}},

        { { -1.0f, -1.0f, -1.0f }, {0.66f, 0.0f} },
        { { -1.0f, -1.0f,  1.0f }, {1.0f , 0.0f} },
        { { -1.0f,  1.0f, -1.0f }, {0.66f, 0.5f} },
        { { -1.0f,  1.0f,  1.0f }, {1.0f , 0.5f} },
        { { -1.0f, -1.0f, -1.0f }, {0.33f, 0.0f} },
        { {  1.0f, -1.0f, -1.0f }, {0.66f, 0.0f} },
        { { -1.0f, -1.0f,  1.0f }, {0.33f, 0.5f} },
        { {  1.0f, -1.0f,  1.0f }, {0.66f, 0.5f} },
        { { -1.0f,  1.0f,  1.0f }, {0.33f, 0.5f} },
        { {  1.0f,  1.0f,  1.0f }, {0.66f, 0.5f} },
        { { -1.0f,  1.0f, -1.0f }, {0.33f, 1.0f} },
        { {  1.0f,  1.0f, -1.0f }, {0.66f, 1.0f} }
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
        VertexData vertex_ = {vertex.position, vertex.texCoord, Eigen::Vector3f::Zero()};

        vertices.push_back(vertex_);
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