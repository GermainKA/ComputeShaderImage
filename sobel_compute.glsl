#version 430

//#ifdef COMPUTE_SHADER

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

// Input & Output Image
layout(binding = 0, rgba32f) readonly uniform image2D inputImage;
layout(binding = 1, rgba32f) writeonly uniform image2D outputImage;

void main() {
     ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imageSize = imageSize(inputImage);

    // Sobel kernels for Gx and Gy
    float kernelGx[3][3] = float[3][3](
        float[3](-1.0,  0.0,  1.0),
        float[3](-2.0,  0.0,  2.0),
        float[3](-1.0,  0.0,  1.0)
    );
    float kernelGy[3][3] = float[3][3](
        float[3](-1.0, -2.0, -1.0),
        float[3]( 0.0,  0.0,  0.0),
        float[3]( 1.0,  2.0,  1.0)
    );

    float gradientX = 0.0;
    float gradientY = 0.0;

    // Compute gradients using Sobel filters
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            ivec2 neighborCoord = pixelCoord + ivec2(x, y);

            // Bounds check
            if (neighborCoord.x < 0 || neighborCoord.x >= imageSize.x ||
                neighborCoord.y < 0 || neighborCoord.y >= imageSize.y) {
                continue;
            }

            // Convert RGB to grayscale: 0.299*R + 0.587*G + 0.114*B
            vec4 neighborColor = imageLoad(inputImage, neighborCoord);
            float intensity = dot(neighborColor.rgb, vec3(0.299, 0.587, 0.114));

            gradientX += intensity * kernelGx[y + 1][x + 1];
            gradientY += intensity * kernelGy[y + 1][x + 1];
        }
    }

    // Gradient magnitude and direction
    float magnitude = sqrt(gradientX * gradientX + gradientY * gradientY);
    float direction = atan(gradientY, gradientX); // Gradient direction in radians

    // Non-maximum suppression (simplified)
    float edgeStrength = magnitude; // Keep the magnitude for now

    // Thresholding
    float lowThreshold = 0.1;  // Example thresholds
    float highThreshold = 0.3;
    float edgeValue = 0.0;

    if (edgeStrength > highThreshold) {
        edgeValue = 1.0; // Strong edge
    } else if (edgeStrength > lowThreshold) {
        edgeValue = 0.5; // Weak edge
    }

    // Write the final edge value
    vec4 outputColor = vec4(edgeValue, edgeValue, edgeValue, 1.0);
    imageStore(outputImage, pixelCoord, outputColor);
}

//#endif