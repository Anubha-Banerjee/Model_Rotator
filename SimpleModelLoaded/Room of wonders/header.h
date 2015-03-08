#include<GLMatrixStack.h>



extern struct component
{
	float x;
	float y;
	float z;
};

int loadMesh(FILE *fp);

extern component *v,*vn,*vt;
extern M3DVector3f *Ver;
extern M3DVector3f *Normals;
extern M3DVector2f *vTexCoords; 	


