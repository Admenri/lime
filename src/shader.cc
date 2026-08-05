#include "src/shader.h"

const std::string kBaseVertexGLSL = R"(
#version 100

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;

uniform mat4 mvp;

varying vec2 fragTexCoord;

void main() {
  fragTexCoord = vertexTexCoord;
  gl_Position = mvp * vec4(vertexPosition, 1.0);
}

)";

// -------------------------------------------------------------

const std::string kSpriteFragmentGLSL = R"(
#version 100

precision mediump float;

varying vec2 fragTexCoord;

uniform sampler2D texture0;

uniform vec4 color;
uniform vec4 tone;
uniform float opacity;

uniform float bushDepth;
uniform float bushOpacity;

void main() {
  vec4 texelColor = texture2D(texture0, fragTexCoord);

  // Tone
  float luma = texelColor.r * 0.299 + texelColor.g * 0.587 + texelColor.b * 0.114;
	texelColor.rgb = mix(texelColor.rgb, vec3(luma), tone.w);
  texelColor.rgb += tone.rgb;

  // Color
  texelColor.rgb = mix(texelColor.rgb, color.rgb, color.a);

  // Opacity
  texelColor *= opacity;

  // Bush effect
  float bushing = float(fragTexCoord.y < bushDepth);
  texelColor.a *= clamp(bushOpacity + bushing, 0.0, 1.0);

  gl_FragColor = texelColor;
}
)";

// -------------------------------------------------------------

const std::string kAlphaTransitionFragmentGLSL = R"(
#version 100

precision mediump float;

varying vec2 fragTexCoord;

uniform sampler2D texture0; // Current
uniform sampler2D texture1; // Frozen

uniform float progress;

void main() {
  vec4 currentColor = texture2D(texture0, fragTexCoord);
  vec4 frozenColor = texture2D(texture1, fragTexCoord);
  gl_FragColor = mix(frozenColor, currentColor, progress);
}
)";

const std::string kMappingTransitionFragmentGLSL = R"(
#version 100

precision mediump float;

varying vec2 fragTexCoord;

uniform sampler2D texture0; // Current
uniform sampler2D texture1; // Frozen
uniform sampler2D texture2; // Mapping

uniform float progress;
uniform float vague;

void main() {
  vec4 currentColor = texture2D(texture0, fragTexCoord);
  vec4 frozenColor = texture2D(texture1, fragTexCoord);
  float mappingValue = texture2D(texture2, fragTexCoord).r;
  float current = clamp(mappingValue, progress, progress + vague);
  gl_FragColor = mix(currentColor, frozenColor, current);
}
)";

// -------------------------------------------------------------

const std::string kViewportFragmentGLSL = R"(
#version 100

precision mediump float;

varying vec2 fragTexCoord;

uniform sampler2D texture0;

uniform vec4 color;
uniform vec4 tone;
uniform float opacity;

void main() {
  vec4 texelColor = texture2D(texture0, fragTexCoord);

  // Tone
  float luma = texelColor.r * 0.299 + texelColor.g * 0.587 + texelColor.b * 0.114;
	texelColor.rgb = mix(texelColor.rgb, vec3(luma), tone.w);
  texelColor.rgb += tone.rgb;

  // Color
  texelColor.rgb = mix(texelColor.rgb, color.rgb, color.a);

  // Opacity
  texelColor *= opacity;

  gl_FragColor = texelColor;
}
)";

// -------------------------------------------------------------

namespace rgssx {

ShaderBase::~ShaderBase() {
  raylib::UnloadShader(shader);
}

SpriteShader::SpriteShader() {
  shader = raylib::LoadShaderFromMemory(kBaseVertexGLSL.c_str(),
                                        kSpriteFragmentGLSL.c_str());

  u_color = raylib::GetShaderLocation(shader, "color");
  u_tone = raylib::GetShaderLocation(shader, "tone");
  u_opacity = raylib::GetShaderLocation(shader, "opacity");
  u_bush_depth = raylib::GetShaderLocation(shader, "bushDepth");
  u_bush_opacity = raylib::GetShaderLocation(shader, "bushOpacity");
}

AlphaTransition::AlphaTransition() {
  shader = raylib::LoadShaderFromMemory(kBaseVertexGLSL.c_str(),
                                        kAlphaTransitionFragmentGLSL.c_str());

  u_frozen_image = raylib::GetShaderLocation(shader, "texture1");
  u_progress = raylib::GetShaderLocation(shader, "progress");
}

MappingTransition::MappingTransition() {
  shader = raylib::LoadShaderFromMemory(kBaseVertexGLSL.c_str(),
                                        kMappingTransitionFragmentGLSL.c_str());

  u_frozen_image = raylib::GetShaderLocation(shader, "texture1");
  u_mapping_image = raylib::GetShaderLocation(shader, "texture2");
  u_progress = raylib::GetShaderLocation(shader, "progress");
  u_vague = raylib::GetShaderLocation(shader, "vague");
}

ViewportShader::ViewportShader() {
  shader = raylib::LoadShaderFromMemory(kBaseVertexGLSL.c_str(),
                                        kViewportFragmentGLSL.c_str());

  u_color = raylib::GetShaderLocation(shader, "color");
  u_tone = raylib::GetShaderLocation(shader, "tone");
  u_opacity = raylib::GetShaderLocation(shader, "opacity");
}

}  // namespace rgssx
