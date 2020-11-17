//
// Created by astrid on 11/17/20.
//

#include "MeshPart.h"

#include <utility>
#include <glad/glad.h>

using namespace glm;

MeshPart::MeshPart(std::shared_ptr<Shape> shape, std::shared_ptr<Program> prog, int index, int tex) :
    shape(std::move(shape)),
    prog(std::move(prog)),
    index(index),
    tex(tex)
{
    auto &posBuf = this->shape->posBuf[index];
    auto &eleBuf = this->shape->eleBuf[index];

    // Calculate centroid
    for (auto it = eleBuf.begin(); it != eleBuf.end();) {
        unsigned int i = *(it++) * 3;
        vec3 v0 = vec3(posBuf[i], posBuf[i + 1], posBuf[i + 2]);
        i = *(it++) * 3;
        vec3 v1 = vec3(posBuf[i], posBuf[i + 1], posBuf[i + 2]);
        i = *(it++) * 3;
        vec3 v2 = vec3(posBuf[i], posBuf[i + 1], posBuf[i + 2]);

        float area = length(cross(v1 - v0, v2 - v0));
        vec3 ct = (v0 + v1 + v2);
        centroid_offset += ct * area;
        surface_area += area;
    }
    centroid_offset /= surface_area * 3.0f;

    // Assuming it's convex, volume is the sum of tetrahedrons
    for (auto it = eleBuf.begin(); it != eleBuf.end();) {
        unsigned int i = *(it++) * 3;
        vec3 v0 = vec3(posBuf[i], posBuf[i + 1], posBuf[i + 2]) - centroid_offset;
        i = *(it++) * 3;
        vec3 v1 = vec3(posBuf[i], posBuf[i + 1], posBuf[i + 2]);
        i = *(it++) * 3;
        vec3 v2 = vec3(posBuf[i], posBuf[i + 1], posBuf[i + 2]);

        mat3 parallelepiped = mat3(v0, v1 - centroid_offset, v2 - centroid_offset);
        volume += abs(determinant(parallelepiped));
    }
    volume /= 2;

    for (auto it = posBuf.begin(); it != posBuf.end();) {
        vec3 v = vec3(*(it++), *(it++), *(it++));
        float radius = length(v);
        inner_radius = min(inner_radius, radius);
    }
}

void MeshPart::draw(glm::mat4 &P, glm::mat4 &V, glm::mat4 &M, vec3 &pos) {
    glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
    glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
    glUniform3fv(prog->getUniform("campos"), 1, &pos[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    shape->draw(prog, false, index);
}
