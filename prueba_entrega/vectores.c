#include "vectores.h"
Vec3 vec_restar(Vec3 a, Vec3 b) { return (Vec3){a.x-b.x,a.y-b.y,a.z-b.z}; }
//Vec3 vec_sumar(Vec3 a, Vec3 b) { return (Vec3){a.x+b.x,a.y+b.y,a.z+b.z}; }
//Vec3 vec_escalar(Vec3 v, double t) { return (Vec3){v.x*t,v.y*t,v.z*t}; }
double vec_punto(Vec3 a, Vec3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
