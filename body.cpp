#include "body.h"

#include <iostream>

namespace pt = boost ::property_tree;
namespace po = boost::program_options;

Body::Body(QOpenGLContext* context): ctx(context)
{
}

Body::Body(QOpenGLContext* context, std::string filename, std::string objectName): ctx(context), name(objectName)
{
    pt ::ptree SettingsTree;

    try {
        read_json(filename, SettingsTree);
    }
    catch (pt ::json_parser_error &e) {
        std ::cout << "Failed to parse the json string.\n" << e.what();
        throw;
    }
    catch (...) {
        std ::cout << "Failed !!!\n";
        throw;
    }
    auto positionIt = SettingsTree.find("position");
    auto velocityIt = SettingsTree.find("velocity");
    auto quaternionIt = SettingsTree.find("quaternion");
    auto angularVelocityIt = SettingsTree.find("angularVelocity");
    auto massIt = SettingsTree.find("mass");
    auto scaleIt = SettingsTree.find("scale");
    auto castsShadowsIt = SettingsTree.find("castsShadows");
    auto modelPathIt = SettingsTree.find("modelPath");

    try {
    if (positionIt == SettingsTree.not_found()
        || velocityIt == SettingsTree.not_found()
        || quaternionIt == SettingsTree.not_found()
        || angularVelocityIt == SettingsTree.not_found()
        || massIt == SettingsTree.not_found()
        || scaleIt == SettingsTree.not_found()
        || castsShadowsIt == SettingsTree.not_found()
        || modelPathIt == SettingsTree.not_found()
        ) throw 1;
    } catch (int) {
        std::cout << "what pos " << (positionIt == SettingsTree.not_found()) << "\n";
        std::cout << "what vel " << (velocityIt == SettingsTree.not_found()) << "\n";
        std::cout << "what quat " << (quaternionIt == SettingsTree.not_found()) << "\n";
        std::cout << "what angularVel " << (angularVelocityIt == SettingsTree.not_found()) << "\n";
        std::cout << "what mass " << (massIt == SettingsTree.not_found()) << "\n";
        std::cout << "what scale " << (scaleIt == SettingsTree.not_found()) << "\n";
        std::cout << "what castsShadows " << (castsShadowsIt == SettingsTree.not_found()) << "\n";
        std::cout << "what modelPath " << (modelPathIt == SettingsTree.not_found()) << "\n";
    }

    m_KinematicParametrs.postition[0] = std::next(positionIt->second.begin(), 0)->second.get_value<double>();
    m_KinematicParametrs.postition[1] = std::next(positionIt->second.begin(), 1)->second.get_value<double>();
    m_KinematicParametrs.postition[2] = std::next(positionIt->second.begin(), 2)->second.get_value<double>();

    m_KinematicParametrs.velocity[0] = std::next(velocityIt->second.begin(), 0)->second.get_value<double>();
    m_KinematicParametrs.velocity[1] = std::next(velocityIt->second.begin(), 1)->second.get_value<double>();
    m_KinematicParametrs.velocity[2] = std::next(velocityIt->second.begin(), 2)->second.get_value<double>();

    m_KinematicParametrs.orientation.w() = std::next(quaternionIt->second.begin(), 0)->second.get_value<double>();
    m_KinematicParametrs.orientation.x() = std::next(quaternionIt->second.begin(), 1)->second.get_value<double>();
    m_KinematicParametrs.orientation.y() = std::next(quaternionIt->second.begin(), 2)->second.get_value<double>();
    m_KinematicParametrs.orientation.z() = std::next(quaternionIt->second.begin(), 3)->second.get_value<double>();

    m_KinematicParametrs.angularVelocity[0] = std::next(angularVelocityIt->second.begin(), 0)->second.get_value<double>();
    m_KinematicParametrs.angularVelocity[1] = std::next(angularVelocityIt->second.begin(), 1)->second.get_value<double>();
    m_KinematicParametrs.angularVelocity[2] = std::next(angularVelocityIt->second.begin(), 2)->second.get_value<double>();

    scale[0] = std::next(scaleIt->second.begin(), 0)->second.get_value<double>();
    scale[1] = std::next(scaleIt->second.begin(), 1)->second.get_value<double>();
    scale[2] = std::next(scaleIt->second.begin(), 2)->second.get_value<double>();

    mass = massIt->second.get_value<double>();

    m_castsShadows = castsShadowsIt->second.get_value<int>();

    ImportModel(modelPathIt->second.data());
}

Body::Body(Body&& body)
{
    using namespace std;
    mass = body.mass;
    m_J = move(body.m_J);
    scale = move(body.scale);
    m_KinematicParametrs = move(body.m_KinematicParametrs);
    m_meshes = move(body.m_meshes);
    m_castsShadows = body.m_castsShadows;
    name = move(body.name);

    ctx = body.ctx;
    body.ctx = nullptr;
}

Body& Body::operator=(Body&& body)
{
    using namespace std;
    mass = body.mass;
    m_J = move(body.m_J);
    scale = move(body.scale);
    m_KinematicParametrs = move(body.m_KinematicParametrs);
    m_meshes = move(body.m_meshes);
    m_castsShadows = body.m_castsShadows;
    name = move(body.name);

    ctx = body.ctx;
    body.ctx = nullptr;
    return*this;
}

Body::Body(const Body& body):m_meshes(body.m_meshes)
{
    mass = body.mass;
    m_J = body.m_J;
    scale = (body.scale);
    m_KinematicParametrs = body.m_KinematicParametrs;
    m_castsShadows = body.m_castsShadows;
    name = body.name;

    ctx = body.ctx;
}

Body& Body::operator=(const Body & body)
{
    mass = body.mass;
    m_J = body.m_J;
    scale = (body.scale);
    m_KinematicParametrs = body.m_KinematicParametrs;
    m_meshes = body.m_meshes;
    m_castsShadows = body.m_castsShadows;
    name = body.name;

    ctx = body.ctx;
    return *this;
}

Body::~Body()
{

}

void Body::draw(QOpenGLShaderProgram& program, RenderType type)
{
    if(!m_castsShadows && type == RenderType::SHADOW)
        return;

    Eigen::Matrix4f m_matrix = getBodyMatrix().transpose();
    QMatrix4x4 m_matrixQt(m_matrix.data());
    program.setUniformValue("world_matrix", m_matrixQt);

    for(auto&& mesh: m_meshes)
        mesh.draw(program);
}

void Body::update()
{
    std::scoped_lock guard(m_mtx);
}

void Body::setBodyPosition(Eigen::Vector3d& pos)
{
    std::scoped_lock guard(m_mtx);
    m_KinematicParametrs.postition = pos;
}

void Body::setBodyPosition(Eigen::Vector3d&& pos)
{
    std::scoped_lock guard(m_mtx);
    m_KinematicParametrs.postition = pos;
}

void Body::translateBody(Eigen::Vector3d& translation)
{
    std::scoped_lock guard(m_mtx);
    m_KinematicParametrs.postition += translation;
}

void Body::setBodyRotation(Eigen::Quaterniond& q)
{
    std::scoped_lock guard(m_mtx);
    m_KinematicParametrs.orientation = q;
}

void Body::rotateBody(Eigen::Quaterniond& q)
{
    std::scoped_lock guard(m_mtx);
    m_KinematicParametrs.orientation = q * m_KinematicParametrs.orientation;
}

void Body::setBodyScale(Eigen::Vector3d& scale)
{
    std::scoped_lock guard(m_mtx);
    this->scale = scale;
}

void Body::setBodyScale(Eigen::Vector3d &&scale)
{
    std::scoped_lock guard(m_mtx);
    this->scale = scale;
}

const Eigen::Vector3d& Body::getBodyPosition() const {
    std::scoped_lock guard(m_mtx);
    return m_KinematicParametrs.postition;
}

Eigen::Vector3d Body::getBodyTranslationMetr() const
{
    std::scoped_lock guard(m_mtx);
    Eigen::Vector3d ret(m_KinematicParametrs.postition);
    return ret * positionToMetr;
}

const Body::BodyKinematicParametrs& Body::getBodyKinematicParametrs()
{
    std::scoped_lock guard(m_mtx);
    return m_KinematicParametrs;
}

void Body::setBodyKinematicParametrs(const BodyKinematicParametrs& param)
{
    std::scoped_lock guard(m_mtx);
    m_KinematicParametrs = param;
}

void Body::setBodyKinematicParametrs(BodyKinematicParametrs&& param)
{
    std::scoped_lock guard(m_mtx);
    m_KinematicParametrs = std::move(param);
}

Eigen::Matrix4f Body::getBodyMatrix() const
{
    std::scoped_lock guard(m_mtx);
    Eigen::Affine3d bodyMatrix;
    bodyMatrix.setIdentity();
    bodyMatrix.translate(m_KinematicParametrs.postition);
    bodyMatrix.rotate(m_KinematicParametrs.orientation);
    bodyMatrix.scale(scale);

    return bodyMatrix.matrix().cast <float>();
}

bool Body::ImportModel(std::string pFile)
{
    Assimp::Importer importer;
    std::filesystem::path path_to_file(pFile);

    std::vector<std::shared_ptr<Material>> materials;

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
        materials.push_back(std::make_shared<Material>(ctx,
                                                       scene_->mMaterials[i],
                                                       path_to_file.parent_path().string())
                                                       );
    }

    if(materials.size() == 0)
    {
        std::cerr << "Unable to load textures\n";
        return false;
    }

    for(uint16_t k = 0; k < scene_->mNumMeshes; k++)
    {
        Mesh mesh(ctx);
        int32_t materialIndex = scene_->mMeshes[k]->mMaterialIndex;
        mesh.m_mat = materials[materialIndex];

        unsigned int texture_index = 0;
        // uint32_t matIndex = std::get<3> (materials[scene_->mMeshes[k]->mMaterialIndex - 1]);

        for(uint32_t i = 0; i < scene_->mMeshes[k]->mNumVertices;i++)
        {
            Mesh::VertexData vertex;
            vertex.position = Eigen::Vector3f(
            scene_->mMeshes[k]->mVertices[i].x,
            scene_->mMeshes[k]->mVertices[i].y,
            scene_->mMeshes[k]->mVertices[i].z
            );

            if(scene_->mMeshes[k]->mTextureCoords[texture_index] != nullptr)
            {
                vertex.colorTextCoord = Eigen::Vector2f(
                    scene_->mMeshes[k]->mTextureCoords[texture_index][i].x,
                    scene_->mMeshes[k]->mTextureCoords[texture_index][i].y
                );
            }

            vertex.normal = Eigen::Vector3f(
            scene_->mMeshes[k]->mNormals[i].x,
            scene_->mMeshes[k]->mNormals[i].y,
            scene_->mMeshes[k]->mNormals[i].z
            );
            mesh.m_vertices.push_back(vertex);
        }

        for(uint32_t i = 0; i < scene_->mMeshes[k]->mNumFaces;i++)
        {
            for(uint32_t j = 0; j < scene_->mMeshes[k]->mFaces[i].mNumIndices; j++)
            {
                mesh.m_indices.push_back(scene_->mMeshes[k]->mFaces[i].mIndices[j]);
            }
        }

        mesh.m_arrayBuf->bind();
        mesh.m_arrayBuf->allocate(mesh.m_vertices.data(), mesh.m_vertices.size() * sizeof(mesh.m_vertices[0]));

        mesh.m_indexBuf->bind();
        mesh.m_indexBuf->allocate(mesh.m_indices.data(), mesh.m_indices.size() * sizeof(mesh.m_indices[0]));

        m_meshes.push_back(std::move(mesh));
    }

    return true;
}

bool Body::ImportTestModel()
{
    Mesh mesh(ctx);

    std::vector <Mesh::VertexData> cubeVertices = {
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

    return true;
}

void Mesh::draw(QOpenGLShaderProgram &program)
{
    m_arrayBuf->bind();
    m_indexBuf->bind();
    m_mat->bindTexture(program);

    // int vertexLocation = program.attributeLocation("a_position");
    int vertexLocation = 0;
    program.enableAttributeArray(vertexLocation);
    program.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(m_vertices[0]));

    int texture_coordinate_location = 1;
    program.enableAttributeArray(texture_coordinate_location);
    program.setAttributeBuffer(texture_coordinate_location, GL_FLOAT, offsetof(VertexData, colorTextCoord), 2, sizeof(m_vertices[0]));

    int normal_location = 2;
    program.enableAttributeArray(normal_location);
    program.setAttributeBuffer(normal_location, GL_FLOAT, offsetof(VertexData, normal), 3, sizeof(m_vertices[0]));


    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);

    m_mat->releaseTexture();
}
