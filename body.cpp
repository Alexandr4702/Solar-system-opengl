#include "body.h"

#include <iostream>

Body::Body(QOpenGLContext* context): ctx(context)
{
    // ImportTestModel();
}

Body::Body(QOpenGLContext* context, std::string filename): ctx(context)
{

    ImportModel(filename);
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
    meshes = move(body.meshes);

    ctx = body.ctx;
    body.ctx = nullptr;
}

Body& Body::operator=(Body&& body)
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
    meshes = move(body.meshes);

    ctx = body.ctx;
    body.ctx = nullptr;
    return*this;
}

Body::Body(const Body& body):meshes(body.meshes)
{
    mass = body.mass;
    J = body.J;
    scale = (body.scale);
    postition = (body.postition);
    velocity = (body.velocity);
    acceleration = (body.acceleration);
    orientation = (body.orientation);
    angularVelocity = (body.angularVelocity);
    angularAcceleration = (body.angularAcceleration);

    ctx = body.ctx;
}

Body& Body::operator=(const Body & body)
{
    mass = body.mass;
    J = body.J;
    scale = (body.scale);
    postition = (body.postition);
    velocity = (body.velocity);
    acceleration = (body.acceleration);
    orientation = (body.orientation);
    angularVelocity = (body.angularVelocity);
    angularAcceleration = (body.angularAcceleration);
    meshes = body.meshes;

    ctx = body.ctx;
    return *this;
}

Body::~Body()
{

}

void Body::draw(QOpenGLShaderProgram& program)
{
    for(auto&& mesh: meshes)
        mesh.draw(program);
}

void Body::draw(QOpenGLShaderProgram& program, Eigen::Matrix4f& matrixCam)
{
    Eigen::Matrix4f m_matrix = getBodyMatrix().transpose();
    QMatrix4x4 m_matrixQt(m_matrix.data());
    program.setUniformValue("m_matrix", m_matrixQt);

    Eigen::Matrix4f vp_matrix = matrixCam.transpose();
    QMatrix4x4 vp_matrixQt(vp_matrix.data());
    program.setUniformValue("vp_matrix", vp_matrixQt);

    draw(program);
}

void Body::update()
{
    std::scoped_lock guard(mtx);
}

void Body::setBodyPosition(Eigen::Vector3d& pos)
{
    std::scoped_lock guard(mtx);
    postition = pos;
}

void Body::setBodyPosition(Eigen::Vector3d&& pos)
{
    std::scoped_lock guard(mtx);
    postition = pos;
}

void Body::translateBody(Eigen::Vector3d& translation)
{
    std::scoped_lock guard(mtx);
    postition += translation;
}

void Body::setBodyRotation(Eigen::Quaterniond& q)
{
    std::scoped_lock guard(mtx);
    orientation = q;
}

void Body::rotateBody(Eigen::Quaterniond& q)
{
    std::scoped_lock guard(mtx);
    orientation = q * orientation;
}

void Body::setBodyScale(Eigen::Vector3d& scale)
{
    std::scoped_lock guard(mtx);
    this->scale = scale;
}

void Body::setBodyScale(Eigen::Vector3d &&scale)
{
    std::scoped_lock guard(mtx);
    this->scale = scale;
}

Eigen::Vector3d Body::getBodyPosition() const
{
    std::scoped_lock guard(mtx);
    Eigen::Vector3d ret(postition);
    return ret;
}

Eigen::Vector3d Body::getBodyTranslationMetr() const
{
    std::scoped_lock guard(mtx);
    Eigen::Vector3d ret(postition);
    return ret * positionToMetr;
}


Eigen::Matrix4f Body::getBodyMatrix() const
{
    std::scoped_lock guard(mtx);
    Eigen::Affine3d bodyMatrix;
    bodyMatrix.setIdentity();
    bodyMatrix.translate(postition);
    bodyMatrix.rotate(orientation);
    bodyMatrix.scale(scale);

    return bodyMatrix.matrix().cast <float>();
}

bool Body::ImportModel(std::string pFile)
{
    Assimp::Importer importer;
    aiString texture_path;
    std::filesystem::path path_to_file(pFile);

    std::vector< std::tuple<std::shared_ptr<QOpenGLTexture>, unsigned int, QImage>> textures;

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

    for(uint32_t i = 0; i < (scene_->mNumMaterials); i++)
    {
        for(uint32_t j = 0; j < 22; j++)
        {

            std::cout << (scene_->mMaterials[i]->GetTextureCount(static_cast<aiTextureType> (j))) << " " << j << " " << i << " \n";

            if((scene_->mMaterials[i]->GetTextureCount(static_cast<aiTextureType> (j))) > 0)
            {
                unsigned int texture_index;
                scene_->mMaterials[i]->GetTexture(static_cast<aiTextureType> (j), 0, &texture_path, NULL, &texture_index);
                QImage texture_image;
                // std::cout << path_to_file.parent_path().string() << "\n";
                QString path_to_texture = QString(path_to_file.parent_path().string().c_str()) + QString("/") + QString(texture_path.C_Str());
                qDebug() << path_to_texture;
                texture_image = QImage(path_to_texture);

                if (!texture_image.isNull())
                {
                    std::shared_ptr<QOpenGLTexture> texture = std::shared_ptr<QOpenGLTexture> (new QOpenGLTexture(texture_image.mirrored()));
                    textures.push_back({texture, texture_index, texture_image});
                } else{
                    std::cerr << "Unable to load textures\n";
                    return false;
                }
            }
        }
    }

    if(textures.size() == 0)
    {
        std::cerr << "Unable to load textures\n";
        return false;
    }

    for(uint16_t k = 0; k < scene_->mNumMeshes; k++)
    {
        Mesh mesh(ctx);

        mesh.texture = std::get<0> (textures[scene_->mMeshes[k]->mMaterialIndex - 1]);
        unsigned int texture_index = std::get<1> (textures[scene_->mMeshes[k]->mMaterialIndex - 1]);
        mesh.raw_texture = std::get<2> (textures[scene_->mMeshes[k]->mMaterialIndex - 1]);

        for(uint32_t i = 0; i < scene_->mMeshes[k]->mNumVertices;i++)
        {
            VertexData vertex;
            vertex.position = Eigen::Vector3f(
            scene_->mMeshes[k]->mVertices[i].x,
            scene_->mMeshes[k]->mVertices[i].y,
            scene_->mMeshes[k]->mVertices[i].z
            );

            if(scene_->mMeshes[k]->mTextureCoords[texture_index] != nullptr)
            {
                vertex.texCoord = Eigen::Vector2f(
                    scene_->mMeshes[k]->mTextureCoords[texture_index][i].x,
                    scene_->mMeshes[k]->mTextureCoords[texture_index][i].y
                );
            }

            vertex.normal = Eigen::Vector3f(
            scene_->mMeshes[k]->mNormals[i].x,
            scene_->mMeshes[k]->mNormals[i].y,
            scene_->mMeshes[k]->mNormals[i].z
            );
            mesh.vertices.push_back(vertex);
        }

        for(uint32_t i = 0; i < scene_->mMeshes[k]->mNumFaces;i++)
        {
            for(uint32_t j = 0; j < scene_->mMeshes[k]->mFaces[i].mNumIndices; j++)
            {
                mesh.indices.push_back(scene_->mMeshes[k]->mFaces[i].mIndices[j]);
            }
        }

        mesh.arrayBuf->bind();
        mesh.arrayBuf->allocate(mesh.vertices.data(), mesh.vertices.size() * sizeof(mesh.vertices[0]));

        mesh.indexBuf->bind();
        mesh.indexBuf->allocate(mesh.indices.data(), mesh.indices.size() * sizeof(mesh.indices[0]));

        meshes.push_back(std::move(mesh));
    }

    return true;
}

bool Body::ImportTestModel()
{
    Mesh mesh(ctx);

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

    // for (auto&& vertex: cubeVertices)
    // {
    //     VertexData vertex_ = {vertex.position, vertex.texCoord, Eigen::Vector3f::Zero()};

    //     vertices.push_back(vertex_);
    // }

    // for (auto&& index: cubeIndices)
    // {
    //     indices.push_back(index);
    // }

    // arrayBuf->bind();
    // arrayBuf->allocate(vertices.data(), vertices.size() * sizeof(vertices[0]));

    // indexBuf->bind();
    // indexBuf->allocate(indices.data(), indices.size() * sizeof(indices[0]));
    return true;
}

void Mesh::draw(QOpenGLShaderProgram &program)
{
    if(texture.get() != nullptr)
        texture->bind();

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
