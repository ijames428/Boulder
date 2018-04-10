uniform sampler2D texture;
 
uniform float timer;
uniform float x_magnitude;
uniform float y_magnitude;
 
void main() {
    vec2 coord = gl_TexCoord[0].xy;
    coord.x += sin(radians(timer + coord.y * 500)) * x_magnitude;
    coord.y += cos(radians(timer + coord.x * 250)) * y_magnitude;
    vec4 pixel_color = texture2D(texture, coord);
 
    gl_FragColor = pixel_color;
}