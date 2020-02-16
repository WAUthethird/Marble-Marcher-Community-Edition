//constants
#define PI 3.14159265
#define SQRT2 1.4142135
#define SQRT3 1.7320508
#define SQRT5 2.2360679

//distance estimator short names
#define COL col_scene
#define DE de_scene

//ray marching 
#define MAX_DIST 500
#define MIN_DIST 1e-6
#define MAX_MARCHES 256
#define NORMARCHES 3
#define overrelax 1.25

//world

//path tracing 
#define MAX_BOUNCE 4.
#define GI_SCALE 8
//to get a better path traced result comment this
#define LIGHT_FIELD_DENOISE
//and make this 0.0001
#define TXAAstr 0.4


//ambient occlusion
#define AMBIENT_MARCHES 4

//shadow sharpness
#define LIGHT_ANGLE 0.08

//random number generator constants
#define PERLIN_SCALE 3

//atmosphere constants
const float Br = 0.0025;
const float Bm = 0.0003;
const float g =  0.9800;
const vec3 nitrogen = vec3(0.650, 0.570, 0.475);
const vec3 Kr = Br / pow(nitrogen, vec3(4.0));
const vec3 Km = Bm / pow(nitrogen, vec3(0.84));
