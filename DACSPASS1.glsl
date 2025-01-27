#version 430

//#ifdef COMPUTE_SHADER

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;


layout(binding = 0, rgba32f) readonly uniform image2D inputImage;
layout(binding = 1, rgba32f) writeonly uniform image2D outputImage;


void main() {
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(inputImage);
    
    if (gid.x >= size.x || gid.y >= size.y) {
        return;
    }

    if ((gid.x % 4 == 0) && (gid.y % 4 == 0))
    {
        vec4 color = imageLoad(inputImage, gid);
        
        imageStore(outputImage, gid, color);
    }
    else
    {
        imageStore(outputImage, gid, vec4(0.0));
    }

}

//#endif