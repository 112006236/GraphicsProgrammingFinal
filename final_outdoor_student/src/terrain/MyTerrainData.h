#pragma once

#include <fstream>
#include <algorithm>
#include <glm\mat4x4.hpp>

class MyTerrainData {
  public:
    MyTerrainData() {
        this->m_worldVtoElevationUVMat = glm::mat4(1.0);
    }

    virtual ~MyTerrainData() {}

  public:
    int m_elevationMapWidth = -1;
    int m_elevationMapHeight = -1;
    float* m_elevationMap = nullptr;

    int m_normalMapWidth = -1;
    int m_normalMapHeight = -1;
    float* m_normalMap = nullptr;

    int m_albedoMapWidth = -1;
    int m_albedoMapHeight = -1;
    int m_albedoMapChannel = -1;
    float* m_albedoMap = nullptr;

    int m_numChunkVertex = -1;
    int m_numChunkIndex = -1;
    float* m_chunkVertices = nullptr;
    unsigned int* m_chunkIndices = nullptr;

    glm::mat4 m_worldVtoElevationUVMat;

  public:
    static MyTerrainData* fromMYTD(const std::string& fileFullpath) {
        std::ifstream input(fileFullpath, std::ios::binary);

        if (!input.is_open()) {
            return nullptr;
        }

        MyTerrainData* mtd = new MyTerrainData();
        int sizeInfo[2];

        input.read((char*)sizeInfo, sizeof(int) * 2);
        mtd->m_elevationMapWidth = sizeInfo[0];
        mtd->m_elevationMapHeight = sizeInfo[1];
        mtd->m_normalMapWidth = sizeInfo[0];
        mtd->m_normalMapHeight = sizeInfo[1];
        mtd->m_albedoMapWidth = sizeInfo[0];
        mtd->m_albedoMapHeight = sizeInfo[1];

        mtd->m_elevationMap = new float[sizeInfo[0] * sizeInfo[1] * 4];
        mtd->m_normalMap = new float[sizeInfo[0] * sizeInfo[1] * 4];
        mtd->m_albedoMap = new float[sizeInfo[0] * sizeInfo[1] * 4];

        // elevation map
        input.read((char*)(mtd->m_elevationMap), sizeof(float) * (sizeInfo[0] * sizeInfo[1]) * 4);
        // normal map
        input.read((char*)(mtd->m_normalMap), sizeof(float) * (sizeInfo[0] * sizeInfo[1]) * 4);
        // color map
        input.read((char*)(mtd->m_albedoMap), sizeof(float) * (sizeInfo[0] * sizeInfo[1]) * 4);

        input.close();

        return mtd;
    }

    bool loadChunkDataFromFile(const std::string& fileFullpath) {
        std::ifstream input(fileFullpath, std::ios::binary);
        int NUM_VERTEX = -1;
        input.read((char*)(&NUM_VERTEX), sizeof(int));
        if (NUM_VERTEX <= 0) {
            input.close();
            return false;
        }
        float* vertices = new float[NUM_VERTEX * 3];
        input.read((char*)vertices, sizeof(float) * NUM_VERTEX * 3);
        int NUM_INDEX = -1;
        input.read((char*)(&NUM_INDEX), sizeof(int));
        if (NUM_INDEX <= 0) {
            input.close();
            return false;
        }
        unsigned int* indices = new unsigned int[NUM_INDEX];
        input.read((char*)indices, sizeof(unsigned int) * NUM_INDEX);

        input.close();

        this->m_chunkVertices = vertices;
        this->m_chunkIndices = indices;
        this->m_numChunkVertex = NUM_VERTEX;
        this->m_numChunkIndex = NUM_INDEX;

        return true;
    }

  public:
    glm::vec3 MyTerrainData::worldVToHeightMapUV(float x, float z) const {
        glm::vec4 uv = this->m_worldVtoElevationUVMat * glm::vec4(x, 0, z, 1.0);
        uv = glm::clamp(uv, glm::vec4(0), glm::vec4(1));

        return glm::vec3(uv.x, 0.0, uv.z);
    }

    float height(const float x, const float z) const {
        glm::vec3 uv = this->worldVToHeightMapUV(x, z);

        float fx = uv.x * (this->m_elevationMapWidth - 1);
        float fz = uv.z * (this->m_elevationMapHeight - 1);

        int x0 = int(floor(fx)), z0 = int(floor(fz));
        int x1 = std::min(x0 + 1, m_elevationMapWidth - 1),
            z1 = std::min(z0 + 1, m_elevationMapHeight - 1);
        auto at = [&](int a, int b) {
            return m_elevationMap[(b * m_elevationMapWidth + a) * 4];
        };
        float u = fx - x0, v = fz - z0;
        return (1 - u) * (1 - v) * at(x0, z0) + u * (1 - v) * at(x1, z0) +
               (1 - u) * v * at(x0, z1) + u * v * at(x1, z1);
    }
};
