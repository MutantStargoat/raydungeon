uniform float aspect;
uniform mat4 matrix;
uniform mat3 dirmatrix;

#define DIST_THRES	1e-3
#define MAX_ITER	256
#define MAX_STEP	0.1

vec3 raymarch(inout vec3 p, in vec3 dir, out float depth);
vec3 shade(in vec3 p, in vec3 dir, in float dist, in float total_dist);
vec3 backdrop(in vec3 dir);
vec3 texgen(in vec3 p, in vec3 n);
float eval_sdf(in vec3 p);
vec3 eval_grad(in vec3 p, float dist);
vec3 primray(in vec2 uv, out vec3 org);

float boxdist(in vec3 p, in vec3 b);
float sphdist(in vec3 p, in vec3 sp, in float srad);

float fbm(vec3 v, int noct);
float snoise(vec3 v);

float eval_sdf_gen(in vec3 p);


void main()
{
	vec2 uv = gl_TexCoord[0].st;
	vec3 rorg;
	vec3 rdir = primray(uv, rorg);
	float depth;

	gl_FragColor.rgb = raymarch(rorg, rdir, depth);
	gl_FragColor.a = 1.0;

	vec4 projp = gl_ProjectionMatrix * vec4(0.0, 0.0, -depth, 1.0);
	float zval = projp.z / projp.w;

	gl_FragDepth = zval;
}

vec3 raymarch(inout vec3 p, in vec3 dir, out float depth)
{
	float d, total_d = 0.0;

	for(int i=0; i<MAX_ITER; i++) {
		if((d = eval_sdf(p)) <= DIST_THRES) {
			depth = total_d;
			return shade(p, dir, d, total_d);
		}

		d = min(d, MAX_STEP);

		p = p + dir * d;
		total_d += d;
	}

	depth = -gl_DepthRange.far;
	return backdrop(dir);
}

vec3 shade(in vec3 p, in vec3 dir, in float dist, in float total_dist)
{
    const vec3 kd = vec3(1.0, 0.3, 0.1);
    const vec3 ks = vec3(0.7, 0.7, 0.7);
    const vec3 ldir = normalize(vec3(-0.01, 0.5, 0.8));
    const vec3 vdir = vec3(0.0, 0.0, -1.0);

    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    vec3 specular = vec3(0.0, 0.0, 0.0);
    
    vec3 n = eval_grad(p, dist);
	return n * 0.5 + 0.5;
    vec3 hdir = normalize(ldir + vdir);

    float ndotl = max(dot(n, ldir), 0.0);
    float ndoth = max(dot(n, hdir), 0.0);
    
    diffuse += texgen(p, n) * ndotl;
    specular += ks * pow(ndoth, 50.0);
    
    float fog = clamp(300.0 / (total_dist * total_dist), 0.0, 1.0);

    return mix(backdrop(dir), diffuse + specular, fog);
}

vec3 backdrop(in vec3 dir)
{
	return vec3(0.1, 0.1, 0.1);
}

float floortile(in vec3 p)
{
	float tile = max(mod(p.x, 1.0), mod(p.z, 1.0));
	tile = min(smoothstep(0.05, 0.1, 1.0 - tile), smoothstep(0.9, 0.95, tile));

	return tile;
}

vec3 texgen(in vec3 p, in vec3 n)
{
	if(p.y < 0.08) {
		const vec3 tilecol = vec3(0.33, 0.3, 0.29);
		const vec3 gapcol = vec3(0.1, 0.15, 0.05);
		float tile = floortile(p);
		return mix(tilecol, gapcol, tile);
	}

	return vec3(1.0, 1.0, 1.0);
}

float eval_sdf(in vec3 p)
{
	float fbmp = fbm(p, 4);

	// walls
	float d = eval_sdf_gen(p) + fbmp * 0.05;

	// floor
	d = min(d, p.y + floortile(p) * 0.01);

	// ceiling
	d = min(d, 3.0 - p.y + fbmp * 0.02);

	return d;
}

#define DELTA	1e-4
vec3 eval_grad(in vec3 p, float dist)
{
	float dfdx = eval_sdf(p + vec3(DELTA, 0.0, 0.0)) - dist;
	float dfdy = eval_sdf(p + vec3(0.0, DELTA, 0.0)) - dist;
	float dfdz = eval_sdf(p + vec3(0.0, 0.0, DELTA)) - dist;
	return normalize(vec3(dfdx, dfdy, dfdz));
}

vec3 primray(in vec2 uv, out vec3 org)
{
	vec3 origin = (gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
	org = origin;

	vec2 nuv = uv * 2.0 - 1.0;
	vec4 cp_near = vec4(nuv, -1.0, 1.0);

	vec4 vdir = gl_ProjectionMatrixInverse * cp_near;
	vec4 wdir = gl_ModelViewMatrixInverse * vec4(vdir.xyz, 0.0);

	return wdir.xyz;
}

float boxdist(in vec3 p, in vec3 b)
{
	vec3 q = abs(p) - b;
	return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float sphdist(in vec3 p, in vec3 sp, in float srad)
{
	return length(p - sp) - srad;
}

#ifdef TEST_SDF
float eval_sdf_gen(in vec3 p)
{
	return boxdist(p - vec3(14.000000, 0.0, 14.000000), vec3(0.550000, 1.0, 4.550000));
}
#endif

float fbm(vec3 v, int noct)
{
	float val = 0.0;
	float f = 1.0;
	for(int i=0; i<noct; i++) {
		val += snoise(v * f) / f;
		f *= 2.0;
	}
	return val;
}

// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20201014 (stegu)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

vec3 mod289(vec3 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
	return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
	return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{ 
	const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
	const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

	// First corner
	vec3 i  = floor(v + dot(v, C.yyy) );
	vec3 x0 =   v - i + dot(i, C.xxx) ;

	// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0 - g;
	vec3 i1 = min( g.xyz, l.zxy );
	vec3 i2 = max( g.xyz, l.zxy );

	//   x0 = x0 - 0.0 + 0.0 * C.xxx;
	//   x1 = x0 - i1  + 1.0 * C.xxx;
	//   x2 = x0 - i2  + 2.0 * C.xxx;
	//   x3 = x0 - 1.0 + 3.0 * C.xxx;
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
	vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

	// Permutations
	i = mod289(i); 
	vec4 p = permute( permute( permute( 
					i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
				+ i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
			+ i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

	// Gradients: 7x7 points over a square, mapped onto an octahedron.
	// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	vec3  ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);

	vec4 b0 = vec4( x.xy, y.xy );
	vec4 b1 = vec4( x.zw, y.zw );

	//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
	//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
	vec4 s0 = floor(b0)*2.0 + 1.0;
	vec4 s1 = floor(b1)*2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));

	vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
	vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

	vec3 p0 = vec3(a0.xy,h.x);
	vec3 p1 = vec3(a0.zw,h.y);
	vec3 p2 = vec3(a1.xy,h.z);
	vec3 p3 = vec3(a1.zw,h.w);

	//Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

	// Mix final noise value
	vec4 m = max(0.5 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
	m = m * m;
	return 105.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
				dot(p2,x2), dot(p3,x3) ) );
}
