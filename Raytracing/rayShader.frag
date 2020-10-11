uniform sampler2D texture;
uniform sampler2D ground;
uniform vec3 campos;
uniform vec2 rotation;
uniform vec2 fov;
uniform vec2 size;
uniform vec4 spheres[100];
uniform vec4 lights[20];
uniform int sphereCount;
uniform int allSpheresCount;
uniform int lightCount;

const float PI = 3.1415926535;

vec3 VRotateX(vec3 v, float amount) {
	float s = sin(amount);
	float c = cos(amount);
	return vec3(v.x, v.y * c - v.z * s, v.y * s + v.z * c);
}
vec3 VRotateY(vec3 v, float amount) {
	float s = sin(amount);
	float c = cos(amount);
	return vec3(v.x * c + v.z * s, v.y, -v.x * s + v.z * c);
}
float VLength(vec3 v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
vec3 VNormalize(vec3 v)
{
	return v / VLength(v);
}
float VAngleXZ(vec3 a, vec3 b)
{
	float ang = atan(b.z, b.x) - atan(a.z, a.x);
	return ang;
}

vec4 Raycast(vec3 pos, vec3 dir)
{
	dir = VNormalize(dir);
	float largestDist = 1;
	float smallestDist = 9999;
	int drawSphere = 0;
	float minstep = 0.004;

	while (largestDist>0 && smallestDist<99999) {
		largestDist = 0;
		smallestDist = 99998 + allSpheresCount - sphereCount;
		for (int i = 0; i < sphereCount; i++) {
			float dist = VLength(pos - spheres[i].xyz);
			if (spheres[i].w > dist + minstep) {
				largestDist = max(largestDist, spheres[i].w - dist);
			}
			else if(abs(spheres[i].w-dist)<=minstep){
				drawSphere = i;
			}
		}
		for (int i = sphereCount; i < allSpheresCount; i++) {
			float dist = VLength(pos - spheres[i].xyz);
			if (dist > spheres[i].w + minstep) {
				smallestDist = min(smallestDist, dist - spheres[i].w);
			}
			else if(abs(spheres[i].w-dist)<=minstep){
				drawSphere = i;
				largestDist = 0;
			}
		}
		pos += dir * min(largestDist, smallestDist);
		minstep += 0.003;
	}
	
	vec3 rpos = pos - spheres[drawSphere].xyz;
	float ycoord = rpos.y / (0.8 + 0.2 * (abs(rpos.x) + abs(rpos.z)));
	float xcoord = min(abs(rpos.z), abs(rpos.x));
	float brightness = 3.0 / max(VLength(pos - campos), 3.0);
	vec4 c = texture2D(ground, vec2(xcoord, ycoord));
	c.rgb *= brightness;
	return c;
}

void main()
{
	vec4 base = texture2D(texture, gl_TexCoord[0].xy);

	vec3 up = VRotateX(vec3(0,1,0), -rotation.y);
	vec3 forward = VRotateX(vec3(0,0,1), -rotation.y);
	vec3 right = VRotateY(vec3(1,0,0), rotation.x);
	forward = VRotateY(forward, rotation.x);
	up = VRotateY(up, rotation.x);
	
	vec2 angles;
	angles.x = (-fov.x + fov.x / size.x * 2 * gl_FragCoord.x);
	angles.y = (-fov.y + fov.y / size.y * 2 * gl_FragCoord.y);

	vec3 dir = forward + right * angles.x + up * angles.y;

    vec4 pixel = Raycast(campos, dir);

    gl_FragColor = pixel;
}