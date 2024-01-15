/*
    This file is part of the PolarPlotterCore library.
    Copyright (c) 2024 Benjamin Carleski

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "point.h"

Point::Point() {
  this->x = 0;
  this->y = 0;
  this->radius = 0;
  this->azimuth = 0;
}

Point::Point(float radius, float azimuth) {
  this->repoint(radius, azimuth);
}

Point::Point(float x, float y, float radius, float azimuth) {
  this->x = x;
  this->y = y;
  this->radius = radius;
  this->azimuth = azimuth;
}

float Point::getX() const {
  return this->x;
}

float Point::getY() const {
  return this->y;
}

float Point::getRadius() const {
  return this->radius;
}

float Point::getAzimuth() const {
  return this->azimuth;
}

void Point::repoint(float radius, float azimuth) {
  this->x = radius * cos(azimuth);
  this->y = radius * sin(azimuth);
  this->radius = radius;
  this->azimuth = azimuth;
}

void Point::cartesianRepoint(float x, float y) {
  this->x = x;
  this->y = y;
  this->radius = sqrt(x * x + y * y);
  this->azimuth = atan2(y, x);
}

void Point::cloneFrom(Point& other) {
  this->x = other.x;
  this->y = other.y;
  this->radius = other.radius;
  this->azimuth = other.azimuth;
}