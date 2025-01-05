#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

// Les images d'entrée et de sortie
layout(binding = 0, rgba32f) readonly uniform image2D inputImage;
layout(binding = 1, rgba32f) writeonly uniform image2D outputImage;

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    
    // Kernel gaussien 3x3
    float kernel[3][3] = float[3][3](
        float[3](1.0, 2.0, 1.0),
        float[3](2.0, 4.0, 2.0),
        float[3](1.0, 2.0, 1.0)
    );
    float kernelSum = 16.0; // Somme des poids du kernel

    vec4 color = vec4(0.0);
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            ivec2 neighborCoord = pixelCoord + ivec2(x, y);
             // Bounds check to avoid accessing out-of-range pixels
            ivec2 imageSize = imageSize(inputImage);
            if (neighborCoord.x < 0 || neighborCoord.x >= imageSize.x ||
                neighborCoord.y < 0 || neighborCoord.y >= imageSize.y) {
                continue;
            }
           vec4 neighborColor = imageLoad(inputImage, neighborCoord);
            color += neighborColor * kernel[y + 1][x + 1];
        }
    }


    color /= kernelSum;
    color = imageLoad(inputImage, pixelCoord);
    color.a = 1.0;

    imageStore(outputImage, pixelCoord, color);
}
