uniform float aspect;
uniform mat4 matrix;
uniform mat3 dirmatrix;

#define DIST_THRES	1e-3
#define MAX_ITER	250
#define MAX_STEP	1.0

vec3 raymarch(inout vec3 p, in vec3 dir, out float depth);
vec3 shade(in vec3 p, in vec3 dir, in float dist, in float total_dist);
vec3 backdrop(in vec3 dir);
float eval_sdf(in vec3 p);
vec3 eval_grad(in vec3 p, float dist);
vec3 primray(in vec2 uv, out vec3 org);

float boxdist(in vec3 p, in vec3 b);
float sphdist(in vec3 p, in vec3 sp, in float srad);

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
    const vec3 ldir = normalize(vec3(-1.0, 1.0, -1.5));
    const vec3 vdir = vec3(0.0, 0.0, -1.0);

    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    vec3 specular = vec3(0.0, 0.0, 0.0);
    
    vec3 n = eval_grad(p, dist);
	return n * 0.5 + 0.5;
    vec3 hdir = normalize(ldir + vdir);

    float ndotl = max(dot(n, ldir), 0.0);
    float ndoth = max(dot(n, hdir), 0.0);
    
    diffuse += kd * ndotl;
    specular += ks * pow(ndoth, 50.0);
    
    float fog = clamp(300.0 / (total_dist * total_dist), 0.0, 1.0);

    return mix(backdrop(dir), diffuse + specular, fog);
}

vec3 backdrop(in vec3 dir)
{
	return vec3(0.1, 0.1, 0.1);
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

	return normalize(wdir.xyz);
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
