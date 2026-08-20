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

#pragma once

#include "src/common.h"
#include "src/font.h"
#include "src/palette.h"
#include "src/raywarp.h"
#include "src/refptr.h"
#include "src/utility.h"

namespace lime {

class Bitmap : public RefCounted<Bitmap>, public Dispoable {
 public:
  /*-export.begin-*/
  Bitmap(std::string filename);
  Bitmap(int width, int height);
  Bitmap(RefPtr<Bitmap> other);
  ~Bitmap();

  int GetWidth();
  int GetHeight();
  RefPtr<Rect> GetRect();
  void Blt(int x,
           int y,
           RefPtr<Bitmap> src_bitmap,
           RefPtr<Rect> src_rect,
           int opacity = 255);
  void StretchBlt(RefPtr<Rect> dst_rect,
                  RefPtr<Bitmap> src_bitmap,
                  RefPtr<Rect> src_rect,
                  int opacity = 255);
  void FillRect(int x, int y, int width, int height, RefPtr<Color> color);
  void FillRect(RefPtr<Rect> rect, RefPtr<Color> color);
  void GradientFillRect(int x,
                        int y,
                        int width,
                        int height,
                        RefPtr<Color> color1,
                        RefPtr<Color> color2,
                        bool vertical = false);
  void GradientFillRect(RefPtr<Rect> rect,
                        RefPtr<Color> color1,
                        RefPtr<Color> color2,
                        bool vertical = false);
  void Clear();
  void ClearRect(int x, int y, int width, int height);
  void ClearRect(RefPtr<Rect> rect);
  RefPtr<Color> GetPixel(int x, int y);
  void SetPixel(int x, int y, RefPtr<Color> color);
  void HueChange(int hue);
  void Blur();
  void RadialBlur(int angle, int division);
  void DrawText(int x,
                int y,
                int width,
                int height,
                std::string str,
                int align = 0);
  void DrawText(RefPtr<Rect> rect, std::string str, int align = 0);
  RefPtr<Rect> TextSize(std::string str);

  void MaskBlt(RefPtr<Rect> dst_rect,
               RefPtr<Bitmap> src_bitmap,
               RefPtr<Rect> src_rect,
               RefPtr<Bitmap> mask);

  RefPtr<Palette> ToPalette();
  void UpdateWithPalette(RefPtr<Palette> palette);

  void SetFilter(int value);
  void SetWrap(int value);

  void DrawPixel(int x, int y, RefPtr<Color> color);
  void DrawLine(int x1, int y1, int x2, int y2, RefPtr<Color> color);
  void DrawLine(RefPtr<Vector2> start_pos,
                RefPtr<Vector2> end_pos,
                RefPtr<Color> color);
  void DrawLineEx(RefPtr<Vector2> start_pos,
                  RefPtr<Vector2> end_pos,
                  float thick,
                  RefPtr<Color> color);
  void DrawLineStrip(std::vector<RefPtr<Vector2>> points, RefPtr<Color> color);
  void DrawLineBezier(RefPtr<Vector2> start_pos,
                      RefPtr<Vector2> end_pos,
                      float thick,
                      RefPtr<Color> color);
  void DrawLineDashed(RefPtr<Vector2> start_pos,
                      RefPtr<Vector2> end_pos,
                      int dash_size,
                      int space_size,
                      RefPtr<Color> color);
  void DrawTriangle(RefPtr<Vector2> v1,
                    RefPtr<Vector2> v2,
                    RefPtr<Vector2> v3,
                    RefPtr<Color> color);
  void DrawTriangleGradient(RefPtr<Vector2> v1,
                            RefPtr<Vector2> v2,
                            RefPtr<Vector2> v3,
                            RefPtr<Color> c1,
                            RefPtr<Color> c2,
                            RefPtr<Color> c3);
  void DrawTriangleLines(RefPtr<Vector2> v1,
                         RefPtr<Vector2> v2,
                         RefPtr<Vector2> v3,
                         RefPtr<Color> color);
  void DrawTriangleFan(std::vector<RefPtr<Vector2>> points,
                       RefPtr<Color> color);
  void DrawTriangleStrip(std::vector<RefPtr<Vector2>> points,
                         RefPtr<Color> color);
  void DrawRectangle(int x, int y, int width, int height, RefPtr<Color> color);
  void DrawRectangle(RefPtr<Vector2> position,
                     RefPtr<Vector2> size,
                     RefPtr<Color> color);
  void DrawRectangle(RefPtr<Rect> rect, RefPtr<Color> color);
  void DrawRectanglePro(RefPtr<Rect> rect,
                        RefPtr<Vector2> origin,
                        float rotation,
                        RefPtr<Color> color);
  void DrawRectangleGradientV(int x,
                              int y,
                              int width,
                              int height,
                              RefPtr<Color> top,
                              RefPtr<Color> bottom);
  void DrawRectangleGradientH(int x,
                              int y,
                              int width,
                              int height,
                              RefPtr<Color> left,
                              RefPtr<Color> right);
  void DrawRectangleGradientEx(RefPtr<Rect> rect,
                               RefPtr<Color> col1,
                               RefPtr<Color> col2,
                               RefPtr<Color> col3,
                               RefPtr<Color> col4);
  void DrawRectangleLines(int x,
                          int y,
                          int width,
                          int height,
                          RefPtr<Color> color);
  void DrawRectangleLinesEx(RefPtr<Rect> rect,
                            float thick,
                            RefPtr<Color> color);
  void DrawRectangleRounded(RefPtr<Rect> rect,
                            float roundness,
                            int segments,
                            RefPtr<Color> color);
  void DrawRectangleRoundedLines(RefPtr<Rect> rect,
                                 float roundness,
                                 int segments,
                                 RefPtr<Color> color);
  void DrawRectangleRoundedLinesEx(RefPtr<Rect> rect,
                                   float roundness,
                                   int segments,
                                   float thick,
                                   RefPtr<Color> color);
  void DrawPoly(RefPtr<Vector2> center,
                int sides,
                float radius,
                float rotation,
                RefPtr<Color> color);
  void DrawPolyLines(RefPtr<Vector2> center,
                     int sides,
                     float radius,
                     float rotation,
                     RefPtr<Color> color);
  void DrawPolyLinesEx(RefPtr<Vector2> center,
                       int sides,
                       float radius,
                       float rotation,
                       float thick,
                       RefPtr<Color> color);
  void DrawCircle(int center_x,
                  int center_y,
                  float radius,
                  RefPtr<Color> color);
  void DrawCircle(RefPtr<Vector2> center, float radius, RefPtr<Color> color);
  void DrawCircleGradient(RefPtr<Vector2> center,
                          float radius,
                          RefPtr<Color> inner,
                          RefPtr<Color> outer);
  void DrawCircleSector(RefPtr<Vector2> center,
                        float radius,
                        float start_angle,
                        float end_angle,
                        int segments,
                        RefPtr<Color> color);
  void DrawCircleSectorLines(RefPtr<Vector2> center,
                             float radius,
                             float start_angle,
                             float end_angle,
                             int segments,
                             RefPtr<Color> color);
  void DrawCircleLines(int center_x,
                       int center_y,
                       float radius,
                       RefPtr<Color> color);
  void DrawCircleLines(RefPtr<Vector2> center,
                       float radius,
                       RefPtr<Color> color);
  void DrawCircleLinesEx(RefPtr<Vector2> center,
                         float radius,
                         float thick,
                         RefPtr<Color> color);
  void DrawEllipse(int center_x,
                   int center_y,
                   float radius_h,
                   float radius_v,
                   RefPtr<Color> color);
  void DrawEllipse(RefPtr<Vector2> center,
                   float radius_h,
                   float radius_v,
                   RefPtr<Color> color);
  void DrawEllipseLines(int center_x,
                        int center_y,
                        float radius_h,
                        float radius_v,
                        RefPtr<Color> color);
  void DrawEllipseLines(RefPtr<Vector2> center,
                        float radius_h,
                        float radius_v,
                        RefPtr<Color> color);
  void DrawRing(RefPtr<Vector2> center,
                float inner_radius,
                float outer_radius,
                float start_angle,
                float end_angle,
                int segments,
                RefPtr<Color> color);
  void DrawRingLines(RefPtr<Vector2> center,
                     float inner_radius,
                     float outer_radius,
                     float start_angle,
                     float end_angle,
                     int segments,
                     RefPtr<Color> color);

  void DrawSplineLinear(std::vector<RefPtr<Vector2>> points,
                        float thick,
                        RefPtr<Color> color);
  void DrawSplineBasis(std::vector<RefPtr<Vector2>> points,
                       float thick,
                       RefPtr<Color> color);
  void DrawSplineCatmullRom(std::vector<RefPtr<Vector2>> points,
                            float thick,
                            RefPtr<Color> color);
  void DrawSplineBezierQuadratic(std::vector<RefPtr<Vector2>> points,
                                 float thick,
                                 RefPtr<Color> color);
  void DrawSplineBezierCubic(std::vector<RefPtr<Vector2>> points,
                             float thick,
                             RefPtr<Color> color);
  void DrawSplineSegmentLinear(RefPtr<Vector2> p1,
                               RefPtr<Vector2> p2,
                               float thick,
                               RefPtr<Color> color);
  void DrawSplineSegmentBasis(RefPtr<Vector2> p1,
                              RefPtr<Vector2> p2,
                              RefPtr<Vector2> p3,
                              RefPtr<Vector2> p4,
                              float thick,
                              RefPtr<Color> color);
  void DrawSplineSegmentCatmullRom(RefPtr<Vector2> p1,
                                   RefPtr<Vector2> p2,
                                   RefPtr<Vector2> p3,
                                   RefPtr<Vector2> p4,
                                   float thick,
                                   RefPtr<Color> color);
  void DrawSplineSegmentBezierQuadratic(RefPtr<Vector2> p1,
                                        RefPtr<Vector2> c2,
                                        RefPtr<Vector2> p3,
                                        float thick,
                                        RefPtr<Color> color);
  void DrawSplineSegmentBezierCubic(RefPtr<Vector2> p1,
                                    RefPtr<Vector2> c2,
                                    RefPtr<Vector2> c3,
                                    RefPtr<Vector2> p4,
                                    float thick,
                                    RefPtr<Color> color);

  ATTR(RefPtr<Font>, Font);
  ATTR(RefPtr<Bitmap>, ShapeBitmap);
  /*-export.end-*/

 public:
  raylib::RenderTexture2D& handle() { return texture_; }

 private:
  friend class CommonDrawScope;
  friend class ShapeDrawScope;
  void DisposeObject() override;

  raylib::RenderTexture2D texture_ = {};

  RefPtr<Font> font_;
  RefPtr<Bitmap> shape_bitmap_;
};

}  // namespace lime
