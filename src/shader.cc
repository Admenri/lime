#include "shader.h"

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

const std::string kTilemapVertexGLSL = R"(
#version 100

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec4 vertexColor;

uniform mat4 mvp;

uniform vec2 offset;
uniform vec2 animOffset;
uniform float tileSize;
uniform float flashAlpha;

varying vec2 fragTexCoord;
varying vec4 fragColor;

const vec2 kRegularArea = vec2(12.0, 12.0);
const vec4 kWaterfallArea = vec4(12.0, 0.0, 4.0, 12.0);
const vec4 kWaterfallAutotileArea = vec4(12.0, 0.0, 2.0, 6.0);

float posInArea(vec2 pos, vec4 area) {
  return pos.x >= area.x && pos.y >= area.y && pos.x <= area.x + area.z && pos.y <= area.y + area.w ? 1.0 : 0.0;
}

void main() {
  vec3 pos = vertexPosition;
  vec2 uv = vertexTexCoord;
  vec4 color = vertexColor;

  pos.xy += offset;

  float addition1 = (uv.x <= kRegularArea.x * tileSize && uv.y <= kRegularArea.y * tileSize) ? 1.0 : 0.0;
  uv.x += animOffset.x * addition1;

  float addition2 = posInArea(uv, kWaterfallArea * tileSize) - posInArea(uv, kWaterfallAutotileArea * tileSize);
  uv.y += animOffset.y * addition2;

  fragTexCoord = uv;
  fragColor = color;
  fragColor.a = (fragColor.rgb == vec3(0.0)) ? 0.0 : flashAlpha;
  gl_Position = mvp * vec4(pos, 1.0);
}

)";

const std::string kTilemapFragmentGLSL = R"(
#version 100

precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;

void main() {
  vec4 texelColor = texture2D(texture0, fragTexCoord);
  gl_FragColor.rgb = mix(texelColor.rgb, fragColor.rgb, fragColor.a);
  gl_FragColor.a = texelColor.a;
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

TilemapShader::TilemapShader() {
  shader = raylib::LoadShaderFromMemory(kTilemapVertexGLSL.c_str(),
                                        kTilemapFragmentGLSL.c_str());

  u_offset = raylib::GetShaderLocation(shader, "offset");
  u_anim_offset = raylib::GetShaderLocation(shader, "animOffset");
  u_tile_size = raylib::GetShaderLocation(shader, "tileSize");
  u_flash_alpha = raylib::GetShaderLocation(shader, "flashAlpha");
}

}  // namespace rgssx
