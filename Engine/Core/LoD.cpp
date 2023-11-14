#include "../../include_all.h"
#include <queue>

const float LOD_DISTANCE_HIGH = 10.0f;
const float LOD_DISTANCE_MEDIUM = 25.0f;
const float LOD_DISTANCE_LOW = 35.0f;

typedef struct Cluster
{
    Color color;
    int lodLevel;
    // std::vector<Entity> entities;
};

struct VertexPair {
    int v1;
    int v2;
};

// Define a struct to represent the result of mesh simplification
struct VertexIndices {
    std::vector<int> indices;
    std::vector<Vector3> vertices;
};

// Helper functions
float Vector4DotProduct(const Vector4& v1, const Vector4& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

Vector4 Vector4Transform(const Vector4& v, const Matrix& mat) {
    Vector4 result;

    result.x = v.x * mat.m0 + v.y * mat.m1 + v.z * mat.m2 + v.w * mat.m3;
    result.y = v.x * mat.m4 + v.y * mat.m5 + v.z * mat.m6 + v.w * mat.m7;
    result.z = v.x * mat.m8 + v.y * mat.m9 + v.z * mat.m10 + v.w * mat.m11;
    result.w = v.x * mat.m12 + v.y * mat.m13 + v.z * mat.m14 + v.w * mat.m15;

    return result;
}

Vector4 Vector4Add(Vector4 v1, Vector4 v2)
{
    Vector4 result = { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z , v1.w + v2.w };

    return result;
}


float determinant3x3(float a, float b, float c, float d, float e, float f, float g, float h, float i) {
    return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
}

Matrix Matrix4Invert(const Matrix& mat) {
    Matrix result;

    // Calculate the determinant of the 4x4 matrix
    float det = mat.m0 * determinant3x3(mat.m5, mat.m6, mat.m7, mat.m9, mat.m10, mat.m11, mat.m13, mat.m14, mat.m15)
                - mat.m1 * determinant3x3(mat.m4, mat.m6, mat.m7, mat.m8, mat.m10, mat.m11, mat.m12, mat.m14, mat.m15)
                + mat.m2 * determinant3x3(mat.m4, mat.m5, mat.m7, mat.m8, mat.m9, mat.m11, mat.m12, mat.m13, mat.m15)
                - mat.m3 * determinant3x3(mat.m4, mat.m5, mat.m6, mat.m8, mat.m9, mat.m10, mat.m12, mat.m13, mat.m14);

    if (std::fabs(det) < 1e-8) {
        // Matrix is singular, cannot invert
        // Handle this case appropriately for your application
        // For now, just return an identity matrix
        return Matrix{1.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 1.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 1.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f};
    }

    // Calculate the inverse of the 4x4 matrix
    float invDet = 1.0f / det;

    result.m0 = determinant3x3(mat.m5, mat.m6, mat.m7, mat.m9, mat.m10, mat.m11, mat.m13, mat.m14, mat.m15) * invDet;
    // (similar calculations for other elements)

    return result;
}


float CalculateQuadricError(const Mesh& mesh, int v1, int v2, const Matrix& quadric) {
    // Get the positions of vertices v1 and v2
    Vector4 posV1{mesh.vertices[v1 * 3], mesh.vertices[v1 * 3 + 1], mesh.vertices[v1 * 3 + 2], 1.0f};
    Vector4 posV2{mesh.vertices[v2 * 3], mesh.vertices[v2 * 3 + 1], mesh.vertices[v2 * 3 + 2], 1.0f};

    // Calculate the error for vertex v1
    float errorV1 = Vector4DotProduct(Vector4Transform(posV1, quadric), posV1);

    // Calculate the error for vertex v2
    float errorV2 = Vector4DotProduct(Vector4Transform(posV2, quadric), posV2);

    // Return the sum of squared distances as the overall error
    return errorV1 + errorV2;
}

void ContractVertices(Mesh& mesh, int v1, int v2, Matrix quadric) {
    // Calculate the optimal position of the contracted vertex
    Vector4 posV1{mesh.vertices[v1 * 3], mesh.vertices[v1 * 3 + 1], mesh.vertices[v1 * 3 + 2], 1.0f};
    Vector4 posV2{mesh.vertices[v2 * 3], mesh.vertices[v2 * 3 + 1], mesh.vertices[v2 * 3 + 2], 1.0f};

    Vector4 newPos = Vector4Add(Vector4Transform(posV1, quadric), Vector4Transform(posV2, quadric));

    // Update the position of v1 to the optimal position
    mesh.vertices[v1 * 3] = newPos.x;
    mesh.vertices[v1 * 3 + 1] = newPos.y;
    mesh.vertices[v1 * 3 + 2] = newPos.z;

    // Remove v2 by setting its position to v1's position
    mesh.vertices[v2 * 3] = newPos.x;
    mesh.vertices[v2 * 3 + 1] = newPos.y;
    mesh.vertices[v2 * 3 + 2] = newPos.z;
}

// Function to simplify the mesh using Quadric Error Metrics
VertexIndices SimplifyMesh(const Mesh& inputMesh, int targetVertexCount) {
    // Copy the input mesh to avoid modifying the original mesh
    Mesh mesh = inputMesh;

    int vertexCount = mesh.vertexCount;

    // Continue simplifying until the vertex count reaches the target
    while (vertexCount > targetVertexCount) {
        // Placeholder variables for the optimal pair
        int optimalV1 = -1;
        int optimalV2 = -1;
        float minError = FLT_MAX;

        Matrix quadric;

        // Iterate through all vertex pairs to find the optimal contraction
        for (int v1 = 0; v1 < vertexCount - 1; ++v1) {
            for (int v2 = v1 + 1; v2 < vertexCount; ++v2) {
                // Create a Quadric matrix for the pair (v1, v2)
                // You need to implement the logic to calculate the Quadric matrix

                // Calculate the error for this contraction
                float error = CalculateQuadricError(mesh, v1, v2, quadric);
                
                // Update the optimal pair if the error is minimal
                if (error < minError) {
                    minError = error;
                    optimalV1 = v1;
                    optimalV2 = v2;
                    vertexCount--;
                }
            }
        }

        // Contract the vertices based on Quadric Error Metrics for the optimal pair
        if (optimalV1 != -1 && optimalV2 != -1) {
            ContractVertices(mesh, optimalV1, optimalV2, quadric);
            // Update mesh properties (vertex count, etc.)
            // You need to implement this part based on the actual mesh structure
            vertexCount--;
        }
    }

    // Create a structure to store the simplified mesh data
    VertexIndices result;

    // Assuming the final simplified mesh is stored in the 'mesh' variable
    result.vertices.reserve(mesh.vertexCount);
    result.indices.reserve(mesh.triangleCount * 3);

    // Copy vertices to the result structure
    for (int i = 0; i < mesh.vertexCount; ++i) {
        Vector3 vertex = { mesh.vertices[i * 3], mesh.vertices[i * 3 + 1], mesh.vertices[i * 3 + 2] };
        result.vertices.push_back(vertex);
    }

    // Copy indices to the result structure
    if (mesh.indices) {
        for (int i = 0; i < mesh.triangleCount * 3; ++i) {
            result.indices.push_back(mesh.indices[i]);
        }
    } else {
        // If the mesh is not indexed, generate new indices
        for (int i = 0; i < mesh.triangleCount * 3; ++i) {
            result.indices.push_back(i);
        }
    }

    // Free the memory used by the original mesh

    return result;
}



// Function to generate a simplified LOD mesh with recalculated texture coordinates
Mesh GenerateLODMesh(VertexIndices meshData, Mesh& sourceMesh) {
    Mesh lodMesh = { 0 };

    if (!meshData.vertices.empty()) {
        int vertexCount = meshData.vertices.size();
        int triangleCount = vertexCount / 3;
        int indexCount = triangleCount * 3;

        // Allocate memory for the new mesh
        lodMesh.vertexCount = vertexCount;
        lodMesh.triangleCount = triangleCount;
        lodMesh.vertices = (float*)malloc(sizeof(float) * 3 * vertexCount);
        lodMesh.indices = (unsigned short*)malloc(sizeof(unsigned short) * indexCount);
        lodMesh.normals = (float*)malloc(sizeof(float) * 3 * vertexCount);
        lodMesh.texcoords = (float*)malloc(sizeof(float) * 2 * vertexCount); // Allocate memory for texcoords
        lodMesh.texcoords2 = (float*)malloc(sizeof(float) * 2 * vertexCount); // Allocate memory for texcoords2
        lodMesh.colors = (unsigned char*)malloc(sizeof(unsigned char) * 4 * vertexCount); // Allocate memory for colors
        lodMesh.tangents = (float*)malloc(sizeof(float) * 4 * vertexCount); // Allocate memory for tangents
        lodMesh.boneWeights = (float*)malloc(sizeof(float) * 4 * vertexCount); // Allocate memory for boneWeights

        // Calculate the bounding box of the contracted mesh
        Vector3 minVertex = meshData.vertices[0];
        Vector3 maxVertex = meshData.vertices[0];
        for (const auto& vertex : meshData.vertices) {
            if (vertex.x < minVertex.x) minVertex.x = vertex.x;
            if (vertex.x > maxVertex.x) maxVertex.x = vertex.x;
            if (vertex.y < minVertex.y) minVertex.y = vertex.y;
            if (vertex.y > maxVertex.y) maxVertex.y = vertex.y;
            if (vertex.z < minVertex.z) minVertex.z = vertex.z;
            if (vertex.z > maxVertex.z) maxVertex.z = vertex.z;
        }

        // Calculate the scaling factors for texture coordinates
        float scaleX = 1.0f / (maxVertex.x - minVertex.x);
        float scaleY = 1.0f / (maxVertex.y - minVertex.y);

        // Assign texture coordinates based on the scaling factors
        for (int i = 0; i < vertexCount; i++) {
            lodMesh.vertices[i * 3] = meshData.vertices[i].x;
            lodMesh.vertices[i * 3 + 1] = meshData.vertices[i].y;
            lodMesh.vertices[i * 3 + 2] = meshData.vertices[i].z;

            lodMesh.texcoords[i * 2] = (meshData.vertices[i].x - minVertex.x) * scaleX;
            lodMesh.texcoords[i * 2 + 1] = (meshData.vertices[i].y - minVertex.y) * scaleY;
        }

        // Generate new indices for non-indexed mesh
        if (sourceMesh.indices) {
            // Allocate memory for the indices
            lodMesh.indices = (unsigned short*)malloc(sizeof(unsigned short) * indexCount);

            // Copy indices from the source mesh
            for (int i = 0; i < indexCount; i++) {
                lodMesh.indices[i] = meshData.indices.at(i);
            }
            // Normalize the normals
            for (int i = 0; i < vertexCount; i++) {
                Vector3 normal = { lodMesh.normals[i * 3], lodMesh.normals[i * 3 + 1], lodMesh.normals[i * 3 + 2] };
                normal = Vector3Normalize(normal);
                lodMesh.normals[i * 3] = normal.x;
                lodMesh.normals[i * 3 + 1] = normal.y;
                lodMesh.normals[i * 3 + 2] = normal.z;
            }
        }
        else {
            lodMesh.indices = sourceMesh.indices;
        }
    }

    // Upload the mesh data to GPU
    UploadMesh(&lodMesh, false);

    // Free the allocated memory before returning
    if (lodMesh.vertices) {
        free(lodMesh.vertices);
        lodMesh.vertices = NULL;
    }
    if (lodMesh.indices) {
        free(lodMesh.indices);
        lodMesh.indices = NULL;
    }
    if (lodMesh.normals) {
        free(lodMesh.normals);
        lodMesh.normals = NULL;
    }
    if (lodMesh.texcoords) {
        free(lodMesh.texcoords);
        lodMesh.texcoords = NULL;
    }
    if (lodMesh.texcoords2) {
        free(lodMesh.texcoords2);
        lodMesh.texcoords2 = NULL;
    }
    if (lodMesh.colors) {
        free(lodMesh.colors);
        lodMesh.colors = NULL;
    }
    if (lodMesh.tangents) {
        free(lodMesh.tangents);
        lodMesh.tangents = NULL;
    }
    if (lodMesh.boneWeights) {
        free(lodMesh.boneWeights);
        lodMesh.boneWeights = NULL;
    }

    return lodMesh;
}
