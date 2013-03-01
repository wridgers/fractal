uniform int iterations;

uniform float red;
uniform float green;
uniform float blue;

uniform float zoom;

uniform vec2 centre;
uniform vec2 resolution;

float fractal(vec2 c) {
	vec2 z 		= vec2(0,0);
	int k 		= 0;
	
	for(int i = 0; i < iterations; i++){
		float zxSq = z.x * z.x;
		float zySq = z.y * z.y;
		
		if (zxSq + zySq > 4.0) break;
		
		float temp = zxSq - zySq + c.x;
		z.y = 2.0*z.x*z.y + c.y;
		z.x = temp;
		
		k++;
	}
	
	float normalised = float(k) + 1.0 - float(log(log(sqrt(z.x * z.x + z.y * z.y))))/float(log(2.0));
	
	return (k == iterations) ? 0.0 : normalised/float(iterations);
}

void main( void ) {
	
	// zero and aspect
	vec2 pos = ( (gl_FragCoord.xy - (resolution.xy / vec2(2,2))) / resolution.xy );
	pos.y 	*= resolution.y/resolution.x;
	
	// zoom
	pos 	*= vec2(1.0/zoom, 1.0/zoom);
	
	// set centre
	pos	+= centre;
	
	gl_FragColor = vec4( fractal(pos)*vec3(0,1,0), 1.0 );

}
