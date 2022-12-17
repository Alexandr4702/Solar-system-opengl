#include "camera.h"

void Camera::setProjetionMatrix(Eigen::Matrix4d mat)
{
    mtx.lock();
    projectiveMatrix = mat;
}

void Camera::setProjetionMatrix(float fovY, float aspectRatio, float zNear, float zFar)
{
    setProjetionMatrix(projective_matrix(fovY, aspectRatio, zNear, zFar));
}

Eigen::Matrix4d Camera::getProjetionMatrix() const
{
    std::scoped_lock guard(mtx);
    return projectiveMatrix;
}

Eigen::Vector3d Camera::getTranslation() const
{
    mtx.lock();
    Eigen::Vector3d ret = -translation;
    mtx.unlock();
    return ret;
}

Eigen::Quaterniond Camera::getRotation() const
{
    mtx.lock();
    Eigen::Quaterniond ret = rotation;
    mtx.unlock();
    return ret;
}

Eigen::Vector3d Camera::getScale() const
{
    mtx.lock();
    Eigen::Vector3d ret = scale;
    mtx.unlock();
    return ret;
}

void Camera::TranslateCam(Eigen::Vector3d& transl)
{
    std::scoped_lock guard(mtx);
    translation += rotation.inverse() * transl;
}

void Camera::TranslateCam(Eigen::Vector3d&& transl)
{
    std::scoped_lock guard(mtx);
    translation += rotation.inverse() * transl;
}

void Camera::setTranslationCam(Eigen::Vector3d& transl)
{
    std::scoped_lock guard(mtx);
    translation = -transl;
}

void Camera::setTranslationCam(Eigen::Vector3d&& transl)
{
    std::scoped_lock guard(mtx);
    translation = -transl;
}

void Camera::rotateCam(Eigen::Quaterniond& rot)
{
    std::scoped_lock guard(mtx);
    rotation = rot * rotation;
}

void Camera::rotateCam(Eigen::Quaterniond&& rot)
{
    std::scoped_lock guard(mtx);
    rotation = rot * rotation;
}

void Camera::setCamRotation(Eigen::Quaterniond& rot)
{
    if(rot.coeffs().hasNaN())
        return;
    std::scoped_lock guard(mtx);
    rotation = rot;
}
void Camera::ScaleCam(Eigen::Vector3d& scal)
{
    std::scoped_lock guard(mtx);
    scale += scal;
}
Eigen::Matrix4d Camera::getCameraMatrix() const
{
    std::scoped_lock guard(mtx);
    Eigen::Affine3d CamMatrix;
    CamMatrix.setIdentity();
    CamMatrix.scale(scale);
    CamMatrix.rotate(rotation);
    CamMatrix.translate(translation);

    return CamMatrix.matrix();
}

Eigen::Matrix4f Camera::getCameraProjectiveMatrix() const
{
    return (projectiveMatrix * getCameraMatrix()).cast <float>();
}

void Camera::printCameraParam(std::ostream& out) const
{
    out << translation.transpose() << "\r\n";
    out << rotation << "\r\n";
}

Eigen::Matrix4d Camera::projective_matrix(float fovY, float aspectRatio, float zNear, float zFar)
{
    float yScale = 1 / tan(fovY * M_PI / 360.0f);
    float xScale = yScale / aspectRatio;

    // float yScale = 1;
    // float xScale = 1;

    Eigen::Matrix4d pmat;
    pmat << xScale, 0, 0, 0,
            0, yScale, 0, 0,
            0, 0, -(zFar+zNear)/(zFar-zNear), -2*zNear*zFar/(zFar-zNear),
            0, 0, -1, 0;
    return pmat;
}

void Camera::StartRotation(Eigen::Vector2d& mouseCoord)
{
    std::scoped_lock guard(mtx);
    mouseCoord0 = mouseCoord;
    q0 = rotation;
}

void Camera::rotateCam(Eigen::Vector2d& mouseCoord)
{
    std::scoped_lock guard(mtx);
    Eigen::Vector2d diff = mouseCoord - mouseCoord0;

    diff *= 0.001;

    float w_rot = sqrt(1- diff.x()*diff.x() - diff.y()*diff.y());
    Eigen::Quaterniond q (w_rot, diff.y(), diff.x(), 0);
    q = q * q0;

    if(q.coeffs().hasNaN())
        return;
    rotation = q;
}

void Camera::setAspectRatio(float ratio)
{
    std::scoped_lock guard(mtx);
    projectiveMatrix = projective_matrix(60.f, ratio, 1e-3f, 4e4f);
    std::cout << projectiveMatrix << "\r\n";
}
