#include "camera.hpp"

#include <limits>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Camera::setOrthographicProjection(
    float left, float right, float top, float bottom, float near, float far)
{
    projectionMatrix = glm::mat4{ 1.0f };
    projectionMatrix[0][0] = 2.f / (right - left);
    projectionMatrix[1][1] = 2.f / (bottom - top);
    projectionMatrix[2][2] = 1.f / (far - near);
    projectionMatrix[3][0] = -(right + left) / (right - left);
    projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
    projectionMatrix[3][2] = -near / (far - near);
}

void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far)
{
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);

    projectionMatrix = glm::perspective(fovy, aspect, near, far);
    projectionMatrix[1] *= -1.f;
}

void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
    viewMatrix = glm::lookAt(position, position + direction, up);
    inverseViewMatrix = glm::inverse(viewMatrix);
}

void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
    viewMatrix = glm::lookAt(position, target, up);
    inverseViewMatrix = glm::inverse(viewMatrix);
}

void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation)
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
    const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
    const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
    viewMatrix = glm::mat4{ 1.f };
    viewMatrix[0][0] = u.x;
    viewMatrix[1][0] = u.y;
    viewMatrix[2][0] = u.z;
    viewMatrix[0][1] = v.x;
    viewMatrix[1][1] = v.y;
    viewMatrix[2][1] = v.z;
    viewMatrix[0][2] = w.x;
    viewMatrix[1][2] = w.y;
    viewMatrix[2][2] = w.z;
    viewMatrix[3][0] = -glm::dot(u, position);
    viewMatrix[3][1] = -glm::dot(v, position);
    viewMatrix[3][2] = -glm::dot(w, position);

    inverseViewMatrix = glm::mat4{ 1.f };
    inverseViewMatrix[0][0] = u.x;
    inverseViewMatrix[0][1] = u.y;
    inverseViewMatrix[0][2] = u.z;
    inverseViewMatrix[1][0] = v.x;
    inverseViewMatrix[1][1] = v.y;
    inverseViewMatrix[1][2] = v.z;
    inverseViewMatrix[2][0] = w.x;
    inverseViewMatrix[2][1] = w.y;
    inverseViewMatrix[2][2] = w.z;
    inverseViewMatrix[3][0] = position.x;
    inverseViewMatrix[3][1] = position.y;
    inverseViewMatrix[3][2] = position.z;
}
