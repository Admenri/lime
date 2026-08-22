// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Admenri Adev <admenri0504@gmail.com>.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
  gl_FragColor = mix(frozenColor, currentColor, current);
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
  float lumin = dot(texelColor.rgb, vec3(0.299, 0.587, 0.114));
	texelColor.rgb = mix(texelColor.rgb, vec3(lumin), tone.w);
  texelColor.rgb += tone.rgb * texelColor.a;

  // Color
  texelColor.rgb = mix(texelColor.rgb, color.rgb * texelColor.a, color.a);

  // Opacity
  texelColor *= opacity;

  // Bush effect
  float bushing = float(fragTexCoord.y < bushDepth);
  texelColor *= clamp(bushOpacity + bushing, 0.0, 1.0);

  gl_FragColor = texelColor;
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
  float lumin = dot(texelColor.rgb, vec3(0.299, 0.587, 0.114));
	texelColor.rgb = mix(texelColor.rgb, vec3(lumin), tone.w);
  texelColor.rgb += tone.rgb * texelColor.a;

  // Color
  texelColor.rgb = mix(texelColor.rgb, color.rgb * texelColor.a, color.a);

  // Opacity
  texelColor *= opacity;

  gl_FragColor = texelColor;
}
)";

// -------------------------------------------------------------

const std::string kBitmapMaskFragmentGLSL = R"(
#version 100

precision mediump float;

varying vec2 fragTexCoord;

uniform sampler2D texture0;

uniform sampler2D mask;

void main() {
  vec4 texelColor = texture2D(texture0, fragTexCoord);
  vec4 maskColor = texture2D(mask, fragTexCoord);

  texelColor *= maskColor.a;
  gl_FragColor = texelColor;
}
)";

// -------------------------------------------------------------

namespace lime {

ShaderBase::~ShaderBase() {
  raylib::UnloadShader(shader);
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

SpriteShader::SpriteShader() {
  shader = raylib::LoadShaderFromMemory(kBaseVertexGLSL.c_str(),
                                        kSpriteFragmentGLSL.c_str());

  u_color = raylib::GetShaderLocation(shader, "color");
  u_tone = raylib::GetShaderLocation(shader, "tone");
  u_opacity = raylib::GetShaderLocation(shader, "opacity");
  u_bush_depth = raylib::GetShaderLocation(shader, "bushDepth");
  u_bush_opacity = raylib::GetShaderLocation(shader, "bushOpacity");
}

ViewportShader::ViewportShader() {
  shader = raylib::LoadShaderFromMemory(kBaseVertexGLSL.c_str(),
                                        kViewportFragmentGLSL.c_str());

  u_color = raylib::GetShaderLocation(shader, "color");
  u_tone = raylib::GetShaderLocation(shader, "tone");
  u_opacity = raylib::GetShaderLocation(shader, "opacity");
}

BitmapMaskShader::BitmapMaskShader() {
  shader = raylib::LoadShaderFromMemory(kBaseVertexGLSL.c_str(),
                                        kBitmapMaskFragmentGLSL.c_str());

  u_mask = raylib::GetShaderLocation(shader, "mask");
}

}  // namespace lime
