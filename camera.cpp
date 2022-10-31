#include "camera.h"

void Camera::setProjetionMatrix(Eigen::Matrix4f mat)
{
    mtx.lock();
    projectiveMatrix = mat;
}

void Camera::setProjetionMatrix(float fovY, float aspectRatio, float zNear, float zFar)
{
    setProjetionMatrix(projective_matrix(fovY, aspectRatio, zNear, zFar));
}

Eigen::Vector3f Camera::getTranslation() const
{
    mtx.lock();
    Eigen::Vector3f ret = translation;
    mtx.unlock();
    return ret;
}

Eigen::Quaternionf Camera::getRotation() const
{
    mtx.lock();
    Eigen::Quaternionf ret = rotation;
    mtx.unlock();
    return ret;
}

Eigen::Vector3f Camera::getScale() const
{
    mtx.lock();
    Eigen::Vector3f ret = scale;
    mtx.unlock();
    return ret;
}

void Camera::TranslateCam(Eigen::Vector3f transl)
{
    std::scoped_lock guard(mtx);
    translation += rotation.inverse() * transl;
}

void Camera::setTranslationCam(Eigen::Vector3f transl)
{
    std::scoped_lock guard(mtx);
    translation = transl;
}

void Camera::rotateCam(Eigen::Quaternionf rot)
{
    std::scoped_lock guard(mtx);
    rotation = rot * rotation;
}
void Camera::setCamRotation(Eigen::Quaternionf& rot)
{
    if(rot.coeffs().hasNaN())
        return;
    std::scoped_lock guard(mtx);
    rotation = rot;
}
void Camera::ScaleCam(Eigen::Vector3f scal)
{
    std::scoped_lock guard(mtx);
    scale += scal;
}
Eigen::Matrix4f Camera::getCameraMatrix() const
{
    std::scoped_lock guard(mtx);
    Eigen::Affine3f CamMatrix;
    CamMatrix.setIdentity();
    CamMatrix.scale(scale);
    CamMatrix.rotate(rotation);
    CamMatrix.translate(translation);

    return CamMatrix.matrix();
}

Eigen::Matrix4f Camera::getCameraProjectiveMatrix() const
{
    return projectiveMatrix * getCameraMatrix();
}

void Camera::printCameraParam(std::ostream& out) const
{
    out << translation.transpose() << "\r\n";
    out << rotation << "\r\n";
}

Eigen::Matrix4f Camera::projective_matrix(float fovY, float aspectRatio, float zNear, float zFar)
{
    float yScale = 1 / tan(fovY * M_PI / 360.0f);
    float xScale = yScale / aspectRatio;

    // float yScale = 1;
    // float xScale = 1;

    Eigen::Matrix4f pmat;
    pmat << xScale, 0, 0, 0,
            0, yScale, 0, 0,
            0, 0, -(zFar+zNear)/(zFar-zNear), -2*zNear*zFar/(zFar-zNear),
            0, 0, -1, 0;
    return pmat;
}