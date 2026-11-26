uniform float iFracScale;
uniform float iFracAng1;
uniform float iFracAng2;
uniform vec3 iFracShift;
uniform vec3 iFracCol;
uniform vec3 iMarblePos;
uniform float iMarbleRad;
uniform float iFlagScale;
uniform vec3 iFlagPos;
uniform int FRACTAL_ITER;
uniform int MARBLE_MODE;
uniform float time;
uniform int iFrame;

uniform float PBR_METALLIC;
uniform float PBR_ROUGHNESS;

uniform vec3 BACKGROUND_COLOR;
uniform vec3 LIGHT_DIRECTION;
uniform vec3 LIGHT_COLOR;
uniform bool SHADOWS_ENABLED; 

uniform float gamma_material;
uniform float gamma_sky;
uniform float gamma_camera;

uniform bool FRACTAL_GLOW;
uniform bool FLAG_GLOW;

//===== Rebuild extensions =====
//fractal rotation matrices (axis + angle, computed on the CPU)
uniform mat3 iFracRot1;
uniform mat3 iFracRot2;

//symmetry
uniform bool SYM_DEFAULT;          //original symmetry => fast path
uniform int SYM_TYPE;            //0 menger, 1 sierpinski, 2 diagonal, 3 none
uniform vec3 SYM_MIRROR;         //1 = mirror (abs) this axis
uniform int SYM_RADIAL_GLOBAL;   //n-fold radial symmetry around Y (once)
uniform int SYM_RADIAL_INTERNAL; //n-fold radial symmetry around Y (every iteration)

//glow
uniform bool GLOW_CUSTOM;        //false = legacy glow colored like the surface
uniform vec3 GLOW_COLOR;
uniform vec3 GLOW_PATTERN;
uniform float GLOW_BRIGHTNESS;
uniform float GLOW_BEAT;         //beat multiplier (1 = no beat sync)
uniform int GLOW_MULTI;          //0 single color, 1 three colors, 2 rainbow, 3 rainbow bands
uniform vec3 GLOW_COLOR2;
uniform vec3 GLOW_COLOR3;
uniform float GLOW_HUE_SPEED;    //rainbow cycles per second
uniform float GLOW_HUE_SPREAD;   //rainbow cycles per marble radius

//sky
uniform bool CUSTOM_SKY;
uniform vec3 SKY_COLOR;

//rubber ball deformation: xyz = contact direction, w = squash amount
uniform vec4 iMarbleSquash;

//multiple flags + checkpoints
#define MAX_EXTRA_FLAGS 16
#define MAX_CHECKPOINTS 32   //drawn checkpoints (nearest to the camera); the game itself has no limit (MAX_DRAWN_CHECKPOINTS in Level.h)
uniform float iGoalState;                  //state of the main (last) flag
uniform int iFlagCount;
uniform vec3 iFlagArr[MAX_EXTRA_FLAGS];
uniform float iFlagState[MAX_EXTRA_FLAGS]; //0 locked, 1 current target, 2 collected
uniform int iCPCount;
uniform vec3 iCPArr[MAX_CHECKPOINTS];
uniform float iCPState[MAX_CHECKPOINTS];   //0 inactive, 1 active

//crush effects
uniform float iMarbleLumps;                //metal "dumpling" surface lumps
#define MAX_SHARDS 24
uniform int iShardCount;
uniform vec4 iShardBound;                  //xyz center, w radius of all shards
uniform vec4 iShardPos[MAX_SHARDS];        //xyz position, w size
uniform vec4 iShardRot[MAX_SHARDS];        //rotation quaternion
uniform vec3 iShardColor;
