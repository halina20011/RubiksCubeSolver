#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define T(x, y, z) (struct Triangle){x, y, z}

struct Triangle{
    float x, y, z;
};

#define FORMAT(var, type, count) do{\
    fwrite(&var, sizeof(type), count, f);\
}while(0);

void save(struct Triangle triangles[], uint32_t size){
    struct Triangle color = T(1, 0, 0);

    FILE *f = fopen("cmodel.bin", "w+");
    if(!f){
        fprintf(stderr, "error while opening file\n");
        return;
    }

    uint32_t byteSize = size * 6;
    FORMAT(byteSize, uint32_t, 1);
    for(size_t i = 0; i < size; i++){
        FORMAT(triangles[i], float, 3);
        FORMAT(color, float, 3);
    }

    fclose(f);
}

int main(){
    struct Triangle t[] = {
        T(0, 0, 0),
        T(1, 0, 0),
        T(1, 1, 0),
    };

    save(t, 3);

    return 0;
}
