#version 430

//#ifdef COMPUTE_SHADER

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;


layout(binding = 0, rgba32f) readonly uniform image2D inputImage;
layout(binding = 1, rgba32f) writeonly uniform image2D outputImage;
layout(binding = 2, rgba32f) readonly uniform image2D ComputeImage;


shared vec4 sharedBlock[11][11] ;

float gray(vec4 c)
{
    // conversion en luma
    return 0.21 * c.r + 0.71 * c.g + 0.08 * c.b;
}

bool isValidPair(int x, int y) {
    // Define valid (lid.x, lid.y) pairs
    const ivec2 validPairs[8] = ivec2[](
        ivec2(0, 2), ivec2(0, 6),ivec2(2, 0), ivec2(2, 4),
        ivec2(4, 2), ivec2(4, 6), ivec2(6, 0), ivec2(6, 4)
    );

    for (int i = 0; i < validPairs.length(); i++) {
        if (validPairs[i].x == x && validPairs[i].y == y) {
            return true;
        }
    }
    return false;
}



void main() {
    float threshold = 1/100;
    // Identifiants locaux et globaux
    ivec2 lid = ivec2(gl_LocalInvocationID.xy);   // ID local dans le work group
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy); // ID global dans l'image
    ivec2 size = imageSize(inputImage);
    
    //On charge tous ce dont on a besoin en memoire partagé
    if (gid.x >= size.x || gid.y >= size.y) {
        return;
    }
    sharedBlock[lid.x+2][lid.y+2] = imageLoad(inputImage, gid);

    if(lid.x == 0 ){
        sharedBlock[0][lid.y] = imageLoad(inputImage, gid - ivec2(0,2));
    }
    if(lid.y == 0 ){
        sharedBlock[lid.x][0] = imageLoad(inputImage, gid - ivec2(2,0));
    }

    if(lid.x == gl_WorkGroupSize.x - 1 ){
        sharedBlock[lid.x+1][lid.y] = imageLoad(inputImage, gid + ivec2(1,0));

    }
    if(lid.y == gl_WorkGroupSize.y - 1 ){
        sharedBlock[lid.x+1][lid.y] = imageLoad(inputImage, gid + ivec2(0,1));

    }
    if((lid.x == gl_WorkGroupSize.x - 1,lid.y == gl_WorkGroupSize.y - 1)){
        sharedBlock[lid.x+1][lid.y] = imageLoad(inputImage, gid + ivec2(1,1));
    }
    //On attend que le chargement se termine
    barrier();

    if(lid.x != lid.y && isValidPair(lid.x, lid.y)){
        vec4 a = sharedBlock[lid.x][lid.y+2] ;
        vec4 b = sharedBlock[lid.x][lid.y-2] ;
        vec4 c = sharedBlock[lid.x+4][lid.y+2] ;
        vec4 d = sharedBlock[lid.x+2][lid.y+4] ;

        vec4 s = (a + b + c + d) * 0.25;
        vec4 ss = (a * a + b * b + c * c + d * d) * 0.25f;

        float v= gray(ss - s*s);

        if (v > threshold) {
            vec4 color = imageLoad(ComputeImage, gid);
            imageStore(outputImage, gid, color);
        } else {
            imageStore(outputImage, gid, s);
        }   

    }

}

//#endif