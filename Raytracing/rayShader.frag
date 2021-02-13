uniform sampler2D ground;
uniform vec3 campos;
uniform vec2 rotation;
uniform vec2 fov;
uniform vec2 size;
uniform vec4 spheres[100];
uniform vec4 uvs[100];
uniform vec4 lights[100];
uniform int sphereCount;
uniform int allSpheresCount;
uniform int lightCount;

const float viewDist = 50.0;
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

vec2 sminPOW(vec3 rayPoint, float k)
{
	float mults = 1.0;
	float adds = 0.0;
	float shortest = 9999999.0;
	float closest = 0;
	
	for (int i = sphereCount; i < allSpheresCount; i++) {
		vec3 tos = spheres[i].xyz-rayPoint;
		float toslen = length(tos) - spheres[i].w;
		closest = step(shortest, toslen) * closest + (1-step(shortest, toslen)) * i;
		shortest = min(shortest, toslen);
		
		float p = pow(toslen, k);
		mults *= p;
		adds += p;
	}
    return vec2(min(shortest, pow(mults/adds, 1.0/k)), closest);
}

float polsmin( float a, float b, float k )
{
	float h = max( k-abs(a-b), 0.0 )/k;
    return min( a, b ) - h*h*k*(1.0/4.0);
}

vec4 Raycast(vec3 pos, vec3 dir, int lit)
{
	dir = normalize(dir);
	int drawSphere = 0;
	float totalDist = 0.0;
	float normalsign = -1.0;

	// Find furthest wall
	for (int j = 0; j < sphereCount*3; j++) {
		int i = j%sphereCount;
		vec3 rpos = pos-spheres[i].xyz;
		int checkstep = int(step(length(rpos), spheres[i].w));

		float b = checkstep * 2.0*dot(rpos, dir);
		float c = checkstep * VLengthS(rpos) - spheres[i].w*spheres[i].w;

		float tosurf = checkstep * (-b + abs(sqrt(b*b - 4*c))) * 0.5;

		drawSphere = checkstep * i + (1-checkstep) * drawSphere;
		
		pos += dir * tosurf;
		totalDist += tosurf;
	}
	
	// Apply texture
	vec3 rpos = pos-spheres[drawSphere].xyz;
	float ycoord = mod(rpos.y / (0.8 + 0.2 * (abs(rpos.x) + abs(rpos.z))), uvs[drawSphere].x) + uvs[drawSphere].w;
	float xcoord = mod(min(abs(rpos.z), abs(rpos.x)), uvs[drawSphere].x) + uvs[drawSphere].z;
	vec4 c = textureLod(ground, vec2(xcoord, ycoord), totalDist*0.05);
	
	float wallDist = totalDist;
	float ballDist = 0;
	float shortest = 999999999;
	float n = 0;
	float smoothDist = 999999999;

	while (ballDist < wallDist && smoothDist > 0.1) {
		vec3 testPos = campos + dir * ballDist;
		smoothDist = 999999999;
		
		for (int j = sphereCount; j < allSpheresCount; j++) {
			float otherDist = distance(testPos, spheres[j]) - spheres[j].w;
			smoothDist = polsmin(smoothDist, otherDist, 0.5);
			n = step(shortest, otherDist) * n + (1-step(shortest, otherDist)) * j;
			shortest = min(shortest, otherDist);
		}
		
		ballDist += smoothDist;
	}
	
	int checkstep = step(0.1, smoothDist);

	vec3 tos = spheres[n] - campos;
	vec3 tosn = tos / ballDist;
	float sangY = asin(dir.y) - asin(tosn.y);
	float sangt = atan(spheres[n].w, ballDist);
	
	vec3 tpos = ((campos + dir * ballDist)-spheres[n].xyz) / spheres[n].w;
	float sangXZ = atan(tpos.z, tpos.x);
	
	// Texture coordinates
	float yc = mod(-0.25 * sangY/sangt + uvs[n].x*0.5, uvs[n].x) + uvs[n].w;
	float xc = mod(0.125 * sangXZ + uvs[n].x*0.5, uvs[n].x) + uvs[n].z;
	vec4 tc = textureLod(ground, vec2(xc, yc), ballDist*0.05);
	
	// Use texture or color based on alpha check
	tc.a = max(lights[n].a, tc.a);
	
	// Set variables to draw this sphere
	c = checkstep * c + (1-checkstep) * tc;
	totalDist = checkstep * totalDist + (1-checkstep) * ballDist;
	drawSphere = checkstep * drawSphere + (1-checkstep) * n;
	pos = checkstep * pos + (1-checkstep) * (campos + dir * ballDist);
	normalsign = checkstep * normalsign + (1-checkstep);
	rpos = checkstep * rpos + (1-checkstep) * tpos * spheres[n].w;

	float brightness = lit * 1.0 / max(totalDist, 1.0);
	
	float lightc = step(sphereCount, drawSphere) * step(drawSphere, sphereCount+lightCount-1);
	
	for(int i=sphereCount; i<sphereCount+lightCount; i++){
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

			shadow *= clamp(sangle/sanglet + 1.0-step(lightshadowdist, tolightlen)*smoothstep(0.0, 0.5, normalMult), 1.0-abs(sign(j-drawSphere)), 1.0);
		}
		
		brightness += 10.0 / tolightlen / tolightlen * max(0.5 + 0.5 * normalMult, 0.0) * shadow;
	}
	
	brightness = lightc*2 + (1.0-lightc) * brightness;
	c.rgb = lights[drawSphere].a*lights[drawSphere].rgb*0.5 + (1.0-lights[drawSphere].a)*c.rgb;
	c.rgb *= clamp(brightness, 0.0, 3.0) + min(-totalDist + viewDist*0.66, 0.0);

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