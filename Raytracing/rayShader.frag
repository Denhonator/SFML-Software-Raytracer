uniform sampler2D ground;
uniform vec3 campos;
uniform vec2 rotation;
uniform vec2 fov;
uniform vec2 size;
uniform vec4 spheres[100];
uniform vec4 lights[100];
uniform int sphereCount;
uniform int allSpheresCount;
uniform int lightCount;

const float viewDist = 40.0;
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
float VLengthS(vec3 v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

float VAngleXZ(vec3 a, vec3 b)
{
	float ang = atan(b.z, b.x) - atan(a.z, a.x);
	return ang;
}

vec4 Raycast(vec3 pos, vec3 dir, int lit)
{
	dir = normalize(dir);
	float largestDist = 1;
	int drawSphere = 0;
	float minstep = 0.004;
	float totalDist = 0;
	float normalsign = -1.0f;

	while (largestDist>0 && totalDist<viewDist) {
		largestDist = 0;
		for (int i = 0; i < sphereCount; i++) {
			float dist = length(pos - spheres[i].xyz);
			if (spheres[i].w > dist + minstep) {
				largestDist = max(largestDist, spheres[i].w - dist);
			}
			else if(abs(spheres[i].w-dist)<=minstep){
				drawSphere = i;
			}
		}
		
		pos += dir * largestDist;
		totalDist += largestDist;
		minstep += largestDist*0.004 - min(-totalDist + viewDist*0.5, 0.0)*0.01;
		minstep *= 1.1;
	}
	
	for (int i = sphereCount; i < allSpheresCount; i++) {
		vec3 tos = spheres[i].xyz-campos;
		float toslen = length(tos);
		float sang = acos(dot(dir, tos/toslen));
		float sangt = atan(spheres[i].w, toslen);
		
		toslen -= spheres[i].w;
		
		int checkstep = int(sign(step(totalDist, toslen) + step(sangt, sang)));
		
		totalDist = checkstep * totalDist + (1-checkstep) * toslen;
		drawSphere = checkstep * drawSphere + (1-checkstep) * i;
		pos = checkstep * pos + (1-checkstep) * (campos + dir * toslen);
		normalsign = checkstep * normalsign + (1-checkstep);
	}
	
	vec3 rpos = pos - spheres[drawSphere].xyz;
	float ycoord = rpos.y / (0.8 + 0.2 * (abs(rpos.x) + abs(rpos.z)));
	float xcoord = min(abs(rpos.z), abs(rpos.x));
	vec4 c = texture2D(ground, vec2(xcoord, ycoord));
	
	float brightness = lit * 1.0 / max(totalDist, 1.0);
	
	float lightc = step(sphereCount, drawSphere) * step(drawSphere, sphereCount+lightCount-1);
	
	for(int i=sphereCount; i<sphereCount+lightCount && lightc==0 && totalDist<viewDist-2.0; i++){
		vec3 tolight = spheres[i].xyz-pos;
		float tolightlen = length(tolight);
		vec3 tolightnorm = tolight/tolightlen;
		float normalMult = length(normalize(rpos*normalsign)+tolightnorm)-1.0;
		
		float shadow = 1.0;
		for(int j=sphereCount+lightCount; j<allSpheresCount; j++){
			float lightshadowdist = distance(spheres[i].xyz, spheres[j].xyz);
			float sanglet = atan(spheres[j].w, lightshadowdist);
			float sangle = acos(dot(-tolightnorm, (spheres[j].xyz-spheres[i].xyz)/lightshadowdist));
			
			float pointshadowdist = 1.5 / (0.8 + 0.2 * distance(pos, spheres[j].xyz));
			sangle = sangle*pointshadowdist - (pointshadowdist-1.0)*sanglet;

			shadow *= clamp(sangle/sanglet, 1.0-abs(sign(j-drawSphere)), 1.0);
		}
		
		vec4 l = lights[i];
		brightness += 40.0 / tolightlen / tolightlen * max(0.5 + 0.5 * normalMult, 0.0) * shadow;
	}
	
	brightness = lightc*2 + (1.0-lightc) * brightness;
	c.rgb = lights[drawSphere].a*lights[drawSphere].rgb*0.5 + (1.0-lights[drawSphere].a)*c.rgb;
	c.rgb *= clamp(brightness, 0.0, 2.0) + min(-totalDist + viewDist*0.66, 0.0);

	return c;
}

void main()
{
	vec3 up = VRotateX(vec3(0,1,0), -rotation.y);
	vec3 forward = VRotateX(vec3(0,0,1), -rotation.y);
	vec3 right = VRotateY(vec3(1,0,0), rotation.x);
	forward = VRotateY(forward, rotation.x);
	up = VRotateY(up, rotation.x);
	
	vec2 angles;
	angles.x = (-fov.x + fov.x / size.x * 2 * gl_FragCoord.x);
	angles.y = (-fov.y + fov.y / size.y * 2 * gl_FragCoord.y);

	vec3 dir = forward + right * angles.x + up * angles.y;

    vec4 pixel = Raycast(campos, dir, 1);

    gl_FragColor = pixel;
}